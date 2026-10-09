#pragma once
// radix_lb の二分探索の代わりに、値の上の桁ごとに xs の範囲の先頭を引く表を作り、範囲の中だけを AVX2 で 8 個ずつ比べて数える版。
// 桁は、xs の最小値と最大値で違う最上位の bit から下へ、範囲が平均 8 個以下になる幅 (18 bit まで) を取る。最上位より上の bit は
// 全要素で同じなので、桁で範囲が決まる。x の順位は、範囲の先頭の位置と、そこから x より小さい要素の個数の和。x は xs にあって
// 範囲の先頭より後ろにあるので、先頭から 16 個ずつ読んで、x 以上の要素が初めて出た位置で止まる (読む先が xs の後ろにはみ出す分は、
// 16 個の番兵を置いておく)。範囲が 32 個を超えるときは、範囲の中を分岐しない二分探索で引く。並べ方は radix_lb と同じ。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
namespace compress_radix_bucket {
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
// 並んだ列 s で、先頭から x より小さい要素の個数。x 以上の要素が列のどこかにあること。
inline u32 count_less(const u32* s, u32 x) {
 const __m256i xv= _mm256_set1_epi32(int(x));
 for(u32 c= 0;; c+= 16) {
  const __m256i v0= _mm256_loadu_si256((const __m256i*)(s + c)), v1= _mm256_loadu_si256((const __m256i*)(s + c + 8));
  // max(v, x) == v の lane は v >= x。
  const u32 m0= u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(_mm256_max_epu32(v0, xv), v0))));
  const u32 m1= u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(_mm256_max_epu32(v1, xv), v1))));
  const u32 m= m0 | m1 << 8;
  if(m) return c + u32(__builtin_ctz(m));
 }
}
inline std::vector<u32> compress(std::vector<u32>& a) {
 const size_t n= a.size();
 if(n <= 256) return compress_small(a);
 auto ob= alloc_huge(n + 16), tb= alloc_huge(n);
 u32* s= ob.get();
 lsd_sort(a.data(), n, s, tb.get());
 const size_t k= std::unique(s, s + n) - s;
 std::vector<u32> xs(s, s + k);
 if(k == 1) {
  std::fill(a.begin(), a.end(), 0u);
  return xs;
 }
 for(int j= 0; j < 16; ++j) s[k + j]= ~0u;  // 番兵
 const int hi= 31 - __builtin_clz(s[0] ^ s[k - 1]);
 int db= 0;
 while(db < 18 && (k >> db) > 8) ++db;
 db= std::min(db, hi + 1);
 const int sh= db ? hi + 1 - db : 0;
 const u32 mk= (1u << db) - 1;
 // tab[h] は、桁の値が h 以上の最初の位置。
 std::vector<u32> tab((size_t(1) << db) + 1);
 {
  size_t h= 0;
  for(size_t j= 0; j < k; ++j) {
   const size_t hj= s[j] >> sh & mk;
   while(h <= hj) tab[h++]= u32(j);
  }
  while(h < tab.size()) tab[h++]= u32(k);
 }
 const u32* t= tab.data();
 for(size_t i= 0; i < n; ++i) {
  const u32 x= a[i], h= x >> sh & mk, p= t[h], q= t[h + 1];
  a[i]= q - p <= 32 ? p + count_less(s + p, x) : p + lower_bound_bl(s + p, q - p, x);
 }
 return xs;
}
}  // namespace compress_radix_bucket
inline std::vector<unsigned> run(std::vector<unsigned>& a) { return compress_radix_bucket::compress(a); }
