#pragma once
// a を写して self-sort-u32 の首位の LSD (lsd_adaptive_hp_cfu) で並べ、重複を除いて xs を作り、各要素を xs から分岐しない二分探索で
// 引いて順位に置き換える。値は 4 byte だけを動かし、添字は持ち歩かない。並べる先と作業用の配列は huge page にし、最初の振り分けは
// a を直接読む。二分探索も huge page の側の写しを引く。256 個以下は std::sort に任せる。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
namespace compress_radix_lb {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline std::unique_ptr<u32, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(u32) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<u32, FreeDeleter>(static_cast<u32*>(p));
}
// a[0, n) を並べた列を out に書く。tmp は作業用で、a は書き換えない。桁の切り方と振り分けは self-sort-u32 の lsd_adaptive_hp_cfu と
// 同じで、全要素の bit or と bit and から要素によって違う bit だけを 11 bit 以下の桁に等分し、全要素が同じ値になる桁は飛ばす。
// 振り分けでは数え上げの表への書き込みを書き先より先に置かせ、GCC にもループを 4 回ずつ展開させる。最初の振り分けは a を直接読み、
// 振り分けの回数の偶奇で書き先の順を決めて、最後が out で終わるようにする。
inline void lsd_sort(const u32* a, size_t n, u32* out, u32* tmp) {
 u32 o= 0, e= ~0u;
 for(size_t i= 0; i < n; ++i) o|= a[i], e&= a[i];
 const u32 diff= o ^ e;
 if(!diff) return (void)std::memcpy(out, a, n * sizeof(u32));  // 全要素が同じ値
 const int lo= __builtin_ctz(diff), hi= 31 - __builtin_clz(diff);
 const int w= hi - lo + 1, p= (w + 10) / 11;
 int sh[3], bits[3];
 for(int d= 0, s= lo; d < p; ++d) {
  bits[d]= w / p + (d < w % p);
  sh[d]= s, s+= bits[d];
 }
 u32 cnt[3][2048];
 for(int d= 0; d < p; ++d) std::memset(cnt[d], 0, sizeof(u32) << bits[d]);
 if(p == 1) {
  const u32 m0= (1u << bits[0]) - 1;
#pragma GCC unroll 4
  for(size_t i= 0; i < n; ++i) ++cnt[0][a[i] >> sh[0] & m0];
 } else if(p == 2) {
  const u32 m0= (1u << bits[0]) - 1, m1= (1u << bits[1]) - 1;
#pragma GCC unroll 4
  for(size_t i= 0; i < n; ++i) {
   const u32 x= a[i];
   ++cnt[0][x >> sh[0] & m0], ++cnt[1][x >> sh[1] & m1];
  }
 } else {
  const u32 m0= (1u << bits[0]) - 1, m1= (1u << bits[1]) - 1, m2= (1u << bits[2]) - 1;
#pragma GCC unroll 4
  for(size_t i= 0; i < n; ++i) {
   const u32 x= a[i];
   ++cnt[0][x >> sh[0] & m0], ++cnt[1][x >> sh[1] & m1], ++cnt[2][x >> sh[2] & m2];
  }
 }
 // 全要素が同じ値になる桁を除き、残りの桁のヒストグラムを先頭からの位置に直す。
 int use[3], k= 0;
 for(int d= 0; d < p; ++d) {
  const u32 sz= 1u << bits[d];
  bool trivial= false;
  for(u32 i= 0; i < sz; ++i)
   if(cnt[d][i]) {
    trivial= cnt[d][i] == n;
    break;
   }
  if(trivial) continue;
  u32 s= 0;
  for(u32 i= 0; i < sz; ++i) {
   u32 t= cnt[d][i];
   cnt[d][i]= s, s+= t;
  }
  use[k++]= d;
 }
 if(!k) return (void)std::memcpy(out, a, n * sizeof(u32));
 const u32* src= a;
 u32* dst= k & 1 ? out : tmp;
 for(int j= 0; j < k; ++j) {
  const int d= use[j], s= sh[d];
  const u32 m= (1u << bits[d]) - 1;
  u32* c= cnt[d];
  asm("" : "+r"(c));  // c が dst と重なるかもしれないと思わせ、下の 2 つの書き込みの順を保たせる
#pragma GCC unroll 4
  for(size_t i= 0; i < n; ++i) {
   const u32 x= src[i], kk= x >> s & m, pos= c[kk];
   c[kk]= pos + 1;
   dst[pos]= x;
  }
  src= dst;
  dst= dst == out ? tmp : out;
 }
}
// clang は二分探索で比較から選ぶところを分岐に直すので、__builtin_unpredictable で cmov のまま残させる。GCC はそのままで cmov にする。
#ifdef __clang__
#define SORT_UNPREDICTABLE(c) __builtin_unpredictable(c)
#else
#define SORT_UNPREDICTABLE(c) (c)
#endif
// 並んだ列 s[0, k) (k >= 1) で x 以上の最初の位置。x が列にあれば、その位置になる。分岐しない二分探索。
inline u32 lower_bound_bl(const u32* s, size_t k, u32 x) {
 const u32* p= s;
 while(k > 1) {
  const size_t half= k >> 1;
  p+= SORT_UNPREDICTABLE(p[half] < x) ? half : 0;
  k-= half;
 }
 return u32(p - s) + (*p < x);
}
// 256 個以下: a を写して std::sort と std::unique で xs を作り、各要素を分岐しない二分探索で引く。
inline std::vector<u32> compress_small(std::vector<u32>& a) {
 std::vector<u32> xs(a);
 std::sort(xs.begin(), xs.end());
 xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
 for(auto& x: a) x= lower_bound_bl(xs.data(), xs.size(), x);
 return xs;
}
inline std::vector<u32> compress(std::vector<u32>& a) {
 const size_t n= a.size();
 if(n <= 256) return compress_small(a);
 auto ob= alloc_huge(n), tb= alloc_huge(n);
 u32* s= ob.get();
 lsd_sort(a.data(), n, s, tb.get());
 const size_t k= std::unique(s, s + n) - s;
 std::vector<u32> xs(s, s + k);
 for(size_t i= 0; i < n; ++i) a[i]= lower_bound_bl(s, k, a[i]);
 return xs;
}
}  // namespace compress_radix_lb
inline std::vector<unsigned> run(std::vector<unsigned>& a) { return compress_radix_lb::compress(a); }
