#pragma once
// LSD 基数ソートで、桁を固定の 11 bit × 5 + 9 bit の 6 つにした版 (self-sort-u32 の lsd11_hp を 64 bit にしたもの)。ヒストグラムを
// 作る走査 1 回で 6 つの桁を数え、全要素が同じ値になる桁は飛ばす。値が 10^9 以下なら上の 3 つの桁が飛び、3 回になる。残る回数が
// 奇数なら a を作業用の配列へ写してから振り分け、a で終える。作業用の配列は huge page にする。鍵は値そのもの。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
namespace sort_lsd11_hp {
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
 if(n <= 256) return std::sort(v.begin(), v.end());
 static constexpr int SH[6]= {0, 11, 22, 33, 44, 55};
 static constexpr u32 SZ[6]= {2048, 2048, 2048, 2048, 2048, 512};
 u32 cnt[6][2048]= {};
 T* a= v.data();
 for(size_t i= 0; i < n; ++i) {
  const u64 k= key(a[i]);
  ++cnt[0][k & 2047], ++cnt[1][k >> 11 & 2047], ++cnt[2][k >> 22 & 2047];
  ++cnt[3][k >> 33 & 2047], ++cnt[4][k >> 44 & 2047], ++cnt[5][k >> 55];
 }
 int use[6], k= 0;
 for(int d= 0; d < 6; ++d) {
  u32* c= cnt[d];
  bool trivial= false;
  for(u32 i= 0; i < SZ[d]; ++i)
   if(c[i]) {
    trivial= c[i] == n;
    break;
   }
  if(trivial) continue;
  u32 s= 0;
  for(u32 i= 0; i < SZ[d]; ++i) {
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
  const int d= use[j], s= SH[d];
  const u64 m= SZ[d] - 1;
  u32* c= cnt[d];
  for(size_t i= 0; i < n; ++i) {
   const T x= src[i];
   dst[c[key(x) >> s & m]++]= x;
  }
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd11_hp
inline void run(std::vector<unsigned long long>& a) { sort_lsd11_hp::sort(a); }
