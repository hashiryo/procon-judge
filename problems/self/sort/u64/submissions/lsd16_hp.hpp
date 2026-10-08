#pragma once
// LSD 基数ソートで、桁を固定の 16 bit × 4 にした版。self-sort-u32 では 16 bit × 2 回が 11 bit × 3 回に負けたが、64 bit では 11 bit の
// 桁だと 6 回振り分けることになるので、4 回に減る分が効くかを見る。ヒストグラムを作る走査 1 回で 4 つの桁を数え (65536 × 4 個、1 MB)、
// 全要素が同じ値になる桁は飛ばす。残る回数が奇数なら a を作業用の配列へ写してから振り分け、a で終える。作業用の配列は huge page に
// する (書き先が 65536 か所に散らばるため)。鍵は値そのもの。4096 個以下は std::sort に任せる。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
namespace sort_lsd16_hp {
using T= unsigned long long;
using u64= unsigned long long;
using u32= unsigned;
// 桁を取り出すときの鍵。鍵の符号なしの順が値の順になるようにする。
inline u64 key(T x) { return x; }
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline std::unique_ptr<T, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(T) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<T, FreeDeleter>(static_cast<T*>(p));
}
inline void sort(std::vector<T>& v) {
 const size_t n= v.size();
 if(n <= 4096) return std::sort(v.begin(), v.end());
 constexpr size_t B= size_t(1) << 16;
 std::vector<u32> cnt(4 * B);
 T* a= v.data();
 for(size_t i= 0; i < n; ++i) {
  const u64 k= key(a[i]);
  ++cnt[k & 0xFFFF], ++cnt[B + (k >> 16 & 0xFFFF)], ++cnt[2 * B + (k >> 32 & 0xFFFF)], ++cnt[3 * B + (k >> 48)];
 }
 int use[4], k= 0;
 for(int d= 0; d < 4; ++d) {
  u32* c= cnt.data() + d * B;
  bool trivial= false;
  for(size_t i= 0; i < B; ++i)
   if(c[i]) {
    trivial= c[i] == n;
    break;
   }
  if(trivial) continue;
  u32 s= 0;
  for(size_t i= 0; i < B; ++i) {
   u32 t= c[i];
   c[i]= s, s+= t;
  }
  use[k++]= d;
 }
 if(!k) return;  // 全要素が同じ値
 auto buf= alloc_huge(n);
 T *src= a, *dst= buf.get();
 if(k & 1) std::memcpy(dst, src, n * sizeof(T)), std::swap(src, dst);
 for(int j= 0; j < k; ++j) {
  const int d= use[j], s= 16 * d;
  u32* c= cnt.data() + d * B;
  for(size_t i= 0; i < n; ++i) {
   const T x= src[i];
   dst[c[key(x) >> s & 0xFFFF]++]= x;
  }
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd16_hp
inline void run(std::vector<unsigned long long>& a) { sort_lsd16_hp::sort(a); }
