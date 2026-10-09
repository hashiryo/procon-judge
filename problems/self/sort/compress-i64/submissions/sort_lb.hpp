#pragma once
// a を self-sort-i64 の首位の msd16_net32 (塊ごとの道を always_inline にした msd16_net32_ai) で並べ、重複を除いて xs を作り、各要素を
// xs から分岐しない二分探索で引いて順位に置き換える。値は 8 byte だけを動かし、添字は持ち歩かない。並べる先は huge page の作業用の
// 配列にして (a は最初の振り分けで読むだけ)、二分探索もそちらを引く。256 個以下は std::sort に任せる。並べ方の説明は
// self-sort-i64 の msd16_net32 にある。
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
#include <limits>
#include <memory>
#include <vector>
namespace compress_sort_lb {
using T= long long;
constexpr bool SIGNED= true;
using u32= unsigned;
using u64= unsigned long long;
// 桁を取り出すときの鍵。鍵の符号なしの順が値の順になるようにする (符号付きなら最上位 bit を反転する)。
inline u64 key(T x) { return SIGNED ? u64(x) ^ (u64(1) << 63) : u64(x); }
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
// 表の m 番目: bit j が立っている lane (軸より大きい) を後ろに、立っていない lane を前に集める vpermd の添字 (64 bit の lane j は
// 32 bit の lane 2j と 2j + 1)。
struct PermTable {
 u32 t[16][8];
 constexpr PermTable(): t() {
  for(int m= 0; m < 16; ++m) {
   int k= 0;
   for(int pass= 0; pass < 2; ++pass)
    for(int j= 0; j < 4; ++j)
     if((m >> j & 1) == pass) t[m][2 * k]= u32(2 * j), t[m][2 * k + 1]= u32(2 * j + 1), ++k;
  }
 }
};
inline constexpr PermTable PERM{};
// 符号付きの比較に直すときに足す値 (符号なしなら最上位 bit を反転する)。
inline __m256i bias() { return SIGNED ? _mm256_setzero_si256() : _mm256_set1_epi64x((long long)(1ull << 63)); }
// a > b の lane が全 bit 1。
inline __m256i gt(__m256i a, __m256i b) {
 if constexpr(SIGNED) return _mm256_cmpgt_epi64(a, b);
 const __m256i s= bias();
 return _mm256_cmpgt_epi64(_mm256_xor_si256(a, s), _mm256_xor_si256(b, s));
}
// x[0, n) を、p 以下を y の前から、p より大きいものを y の後ろから詰めて書き、前に書いた個数を返す。
inline size_t partition(const T* x, T* y, size_t n, T p) {
 const __m256i s= bias();
 const __m256i pv= _mm256_xor_si256(_mm256_set1_epi64x((long long)p), s);
 size_t l= 0, r= n, i= 0;
 for(; i + 8 <= n; i+= 4) {
  const __m256i v= _mm256_loadu_si256((const __m256i*)(x + i));
  const int m= _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpgt_epi64(_mm256_xor_si256(v, s), pv)));
  const __m256i w= _mm256_permutevar8x32_epi32(v, _mm256_loadu_si256((const __m256i*)PERM.t[m]));
  const int nr= __builtin_popcount(m);
  _mm256_storeu_si256((__m256i*)(y + l), w);
  _mm256_storeu_si256((__m256i*)(y + r - 4), w);
  l+= 4 - nr, r-= nr;
 }
 for(; i < n; ++i) {
  if(x[i] <= p) y[l++]= x[i];
  else y[--r]= x[i];
 }
 return l;
}
// a と b の lane ごとの小さい方を a に、大きい方を b に置く。
inline void minmax(__m256i& a, __m256i& b) {
 const __m256i g= gt(a, b);
 const __m256i lo= _mm256_blendv_epi8(a, b, g), hi= _mm256_blendv_epi8(b, a, g);
 a= lo, b= hi;
}
// 距離 1 と 2 の lane どうしを比べ、MASK (32 bit の lane 単位) の bit が立つ lane に大きい方を置く。
template <int MASK> inline __m256i step1(__m256i v) {
 __m256i q= _mm256_permute4x64_epi64(v, 0xB1), lo= v, hi= q;
 minmax(lo, hi);
 return _mm256_blend_epi32(lo, hi, MASK);
}
template <int MASK> inline __m256i step2(__m256i v) {
 __m256i q= _mm256_permute4x64_epi64(v, 0x4E), lo= v, hi= q;
 minmax(lo, hi);
 return _mm256_blend_epi32(lo, hi, MASK);
}
// bitonic な 4 個を昇順にする。
inline __m256i merge4(__m256i v) { return step1<0xCC>(step2<0xF0>(v)); }
// 任意の 4 個を昇順にする。2 個ずつ昇順と降順に並べて bitonic にしてから merge4。
inline __m256i sort4(__m256i v) { return merge4(step1<0x3C>(v)); }
inline __m256i reverse4(__m256i v) { return _mm256_permute4x64_epi64(v, 0x1B); }
// a, b を並べた 8 個が bitonic なら昇順にする。
inline void merge8(__m256i& a, __m256i& b) {
 minmax(a, b);
 a= merge4(a), b= merge4(b);
}
inline void sort8(__m256i& a, __m256i& b) {
 a= sort4(a), b= reverse4(sort4(b));
 merge8(a, b);
}
// a, b, c, d を並べた 16 個が bitonic なら昇順にする。
inline void merge16(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 minmax(a, c), minmax(b, d);
 merge8(a, b), merge8(c, d);
}
inline void sort16(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 sort8(a, b), sort8(c, d);
 // 後ろの 8 個を逆順にして (reverse4(d), reverse4(c)) とつなぐと 16 個が bitonic になる。
 __m256i rc= reverse4(d), rd= reverse4(c);
 minmax(a, rc), minmax(b, rd);
 merge8(a, b), merge8(rc, rd);
 c= rc, d= rd;
}
// 8 本のベクタの 32 個を昇順にする。
inline void sort32(__m256i* v) {
 sort16(v[0], v[1], v[2], v[3]), sort16(v[4], v[5], v[6], v[7]);
 __m256i r4= reverse4(v[7]), r5= reverse4(v[6]), r6= reverse4(v[5]), r7= reverse4(v[4]);
 minmax(v[0], r4), minmax(v[1], r5), minmax(v[2], r6), minmax(v[3], r7);
 merge16(v[0], v[1], v[2], v[3]), merge16(r4, r5, r6, r7);
 v[4]= r4, v[5]= r5, v[6]= r6, v[7]= r7;
}
// a[0, n) (n <= 32) を、4 個ずつのベクタに最大値で埋めて読み、4 / 8 / 16 / 32 個のネットワークで並べて書き戻す。端の半端な
// ベクタは vpmaskmovq で読み書きし、配列の外には触れない。rem はそのベクタから先に残っている個数。
inline __m256i load_pad(const T* a, long long rem) {
 if(rem >= 4) return _mm256_loadu_si256((const __m256i*)a);
 const __m256i m= _mm256_cmpgt_epi64(_mm256_set1_epi64x(rem), _mm256_setr_epi64x(0, 1, 2, 3));
 const __m256i pad= _mm256_set1_epi64x((long long)std::numeric_limits<T>::max());
 return _mm256_blendv_epi8(pad, _mm256_maskload_epi64((const long long*)a, m), m);
}
inline void store_part(T* a, long long rem, __m256i v) {
 if(rem >= 4) return _mm256_storeu_si256((__m256i*)a, v);
 const __m256i m= _mm256_cmpgt_epi64(_mm256_set1_epi64x(rem), _mm256_setr_epi64x(0, 1, 2, 3));
 _mm256_maskstore_epi64((long long*)a, m, v);
}
// src[0, n) (n <= 32) を並べて dst[0, n) に書く (src と dst は同じでもよい)。
inline void small_sort_to(const T* src, T* dst, size_t n) {
 if(n <= 1) {
  if(n) dst[0]= src[0];
  return;
 }
 const long long r= (long long)n;
 if(n <= 4) {
  store_part(dst, r, sort4(load_pad(src, r)));
 } else if(n <= 8) {
  __m256i x= load_pad(src, 4), y= load_pad(src + 4, r - 4);
  sort8(x, y);
  store_part(dst, 4, x), store_part(dst + 4, r - 4, y);
 } else if(n <= 16) {
  __m256i x= load_pad(src, 4), y= load_pad(src + 4, 4), z= load_pad(src + 8, r - 8), w= load_pad(src + 12, r - 12);
  sort16(x, y, z, w);
  store_part(dst, 4, x), store_part(dst + 4, 4, y), store_part(dst + 8, r - 8, z), store_part(dst + 12, r - 12, w);
 } else {
  __m256i v[8];
  for(int j= 0; j < 8; ++j) v[j]= load_pad(src + 4 * j, r - 4 * j);
  sort32(v);
  for(int j= 0; j < 8; ++j) store_part(dst + 4 * j, r - 4 * j, v[j]);
 }
}
inline void small_sort(T* a, size_t n) { small_sort_to(a, a, n); }
inline T median3(T a, T b, T c) { return std::max(std::min(a, b), std::min(std::max(a, b), c)); }
// 8192 個以上の塊は、等間隔に取った 64 個の標本の中央値 (std::nth_element) を軸にする。128 個以上は 9 点、それ未満は 3 点の中央値。
inline T choose_pivot(const T* a, size_t n) {
 if(n >= 8192) {
  T s[64];
  const size_t step= n / 64;
  for(int i= 0; i < 64; ++i) s[i]= a[i * step + step / 2];
  std::nth_element(s, s + 32, s + 64);
  return s[32];
 }
 if(n < 128) return median3(a[0], a[n / 2], a[n - 1]);
 const size_t s= n / 8;
 return median3(median3(a[0], a[s], a[2 * s]), median3(a[3 * s], a[4 * s], a[5 * s]), median3(a[6 * s], a[7 * s], a[n - 1]));
}
// x[0, n) を並べる。y は同じ長さの作業用の領域で、分割のたびに x と y を入れ替える。x_is_a は x が元の配列 a の側かで、
// 並べ終えた値は a の側に置く (a の側でなければ最後に y へ写す)。
inline void rec(T* x, T* y, size_t n, int depth, bool x_is_a) {
 while(n > 32) {
  if(depth-- == 0) {
   std::sort(x, x + n);
   if(!x_is_a) std::memcpy(y, x, n * sizeof(T));
   return;
  }
  const T p= choose_pivot(x, n);
  size_t k= partition(x, y, n, p);
  std::swap(x, y), x_is_a= !x_is_a;
  if(k == n) {
   // 全要素が p 以下なので p は最大値。p と等しい要素 (並べ終わり) を後ろに分け、前だけを続ける。
   if(p == std::numeric_limits<T>::min()) {  // 全要素が最小値
    if(!x_is_a) std::memcpy(y, x, n * sizeof(T));
    return;
   }
   k= partition(x, y, n, p - 1);
   std::swap(x, y), x_is_a= !x_is_a;
   if(!x_is_a) std::memcpy(y + k, x + k, (n - k) * sizeof(T));
   n= k;
   continue;
  }
  // 小さい側を再帰で、大きい側をループで並べる (スタックの深さを log n に抑える)。
  if(k < n - k) rec(x, y, k, depth, x_is_a), x+= k, y+= k, n-= k;
  else rec(x + k, y + k, n - k, depth, x_is_a), n= k;
 }
 small_sort(x, n);
 if(!x_is_a) std::memcpy(y, x, n * sizeof(T));
}
// u32 の鍵のソーティングネットワーク (self-sort-u32 の msd16_net と同じ)。stepD<MASK> は距離 D の lane どうしを比べ、MASK の bit が
// 立つ lane に大きい方を置く。
namespace net32 {
inline __m256i vmin(__m256i a, __m256i b) { return _mm256_min_epu32(a, b); }
inline __m256i vmax(__m256i a, __m256i b) { return _mm256_max_epu32(a, b); }
template <int MASK> inline __m256i step1(__m256i v) {
 __m256i q= _mm256_shuffle_epi32(v, 0xB1);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
template <int MASK> inline __m256i step2(__m256i v) {
 __m256i q= _mm256_shuffle_epi32(v, 0x4E);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
template <int MASK> inline __m256i step4(__m256i v) {
 __m256i q= _mm256_permute4x64_epi64(v, 0x4E);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
// bitonic な 8 個を昇順にする。
inline __m256i merge8(__m256i v) { return step1<0xAA>(step2<0xCC>(step4<0xF0>(v))); }
// 任意の 8 個を昇順にする。2 個ずつ昇順と降順、4 個ずつ昇順と降順に並べて bitonic にしてから merge8。
inline __m256i sort8(__m256i v) { return merge8(step1<0x5A>(step2<0x3C>(step1<0x66>(v)))); }
inline __m256i reverse8(__m256i v) { return _mm256_permutevar8x32_epi32(v, _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0)); }
// a, b を並べた 16 個が bitonic なら昇順にする。
[[gnu::always_inline]] inline void merge16(__m256i& a, __m256i& b) {
 __m256i l= vmin(a, b), h= vmax(a, b);
 a= merge8(l), b= merge8(h);
}
[[gnu::always_inline]] inline void sort16(__m256i& a, __m256i& b) {
 a= sort8(a), b= reverse8(sort8(b));
 merge16(a, b);
}
[[gnu::always_inline]] inline void sort32(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 sort16(a, b), sort16(c, d);
 // 後ろの 16 個を逆順にして (reverse8(d), reverse8(c)) とつなぐと 32 個が bitonic になる。
 __m256i rc= reverse8(d), rd= reverse8(c);
 __m256i l0= vmin(a, rc), h0= vmax(a, rc), l1= vmin(b, rd), h1= vmax(b, rd);
 merge16(l0, l1), merge16(h0, h1);
 a= l0, b= l1, c= h0, d= h1;
}
// a, b, c, d を並べた 32 個が bitonic なら昇順にする。
[[gnu::always_inline]] inline void merge32(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 __m256i l0= vmin(a, c), h0= vmax(a, c), l1= vmin(b, d), h1= vmax(b, d);
 merge16(l0, l1), merge16(h0, h1);
 a= l0, b= l1, c= h0, d= h1;
}
// 8 本のベクタの 64 個を昇順にする。前後の 32 個を並べ、後ろを逆順にしてつないだ bitonic な 64 個を merge する。
[[gnu::always_inline]] inline void sort64(__m256i* v) {
 sort32(v[0], v[1], v[2], v[3]), sort32(v[4], v[5], v[6], v[7]);
 const __m256i r4= reverse8(v[7]), r5= reverse8(v[6]), r6= reverse8(v[5]), r7= reverse8(v[4]);
 __m256i l0= vmin(v[0], r4), h0= vmax(v[0], r4), l1= vmin(v[1], r5), h1= vmax(v[1], r5);
 __m256i l2= vmin(v[2], r6), h2= vmax(v[2], r6), l3= vmin(v[3], r7), h3= vmax(v[3], r7);
 merge32(l0, l1, l2, l3), merge32(h0, h1, h2, h3);
 v[0]= l0, v[1]= l1, v[2]= l2, v[3]= l3, v[4]= h0, v[5]= h1, v[6]= h2, v[7]= h3;
}
}  // namespace net32
// 塊の鍵の作り方。鍵 (key(x) - base) の bit [ks, ks + rk) を上に、塊の中の位置を下の 6 bit に詰める (rk <= 26)。
struct KeyParams {
 __m256i flip, base, mask;  // flip は符号付きのとき最上位 bit、mask は下の rk bit (64 bit の lane ごと)
 __m128i ks;
};
// 塊 s[0, m) の 8t 番目からの 8 個の鍵を作る。値を 4 個ずつ 2 回読み、64 bit のまま鍵の bit を取り出してから、下の 32 bit を元の順に
// 集める。m 個より後ろの lane は最大値で埋める (読むのは s[0, 8t + 8) で、塊の後ろの要素を読んでも鍵には使わない)。
[[gnu::always_inline]] inline __m256i make_keys(const T* s, int m, int t, const KeyParams& kp) {
 const int at= 8 * t;
 const auto part= [&](const T* p) {
  const __m256i v= _mm256_sub_epi64(_mm256_xor_si256(_mm256_loadu_si256((const __m256i*)p), kp.flip), kp.base);
  return _mm256_castsi256_ps(_mm256_and_si256(_mm256_srl_epi64(v, kp.ks), kp.mask));
 };
 // shuffle_ps で 128 bit の中ごとに偶数番目の 32 bit を集めると (y0 y1 y4 y5 | y2 y3 y6 y7) になるので、64 bit 単位で並べ直す。
 const __m256i y= _mm256_permute4x64_epi64(_mm256_castps_si256(_mm256_shuffle_ps(part(s + at), part(s + at + 4), 0x88)), 0xD8);
 const __m256i iota= _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
 const __m256i k= _mm256_or_si256(_mm256_slli_epi32(y, 6), _mm256_add_epi32(iota, _mm256_set1_epi32(at)));
 if(m - at >= 8) return k;
 return _mm256_blendv_epi8(_mm256_set1_epi32(-1), k, _mm256_cmpgt_epi32(_mm256_set1_epi32(m - at), iota));
}
// 並べた鍵のベクタ v[0, nv) の先頭 m 個に、上の 26 bit が等しい隣どうしがあるか。1 lane ずらした鍵と比べる。
[[gnu::always_inline]] inline bool has_tie(const __m256i* v, int nv, int m) {
 const __m256i up= _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6), last= _mm256_set1_epi32(7);
 __m256i carry= _mm256_set1_epi32(-1);  // 先頭の要素の前には、どの鍵とも等しくならない値を置く
 for(int t= 0; t < nv; ++t) {
  const int rem= m - 8 * t;
  if(rem <= 0) break;
  const __m256i p= _mm256_srli_epi32(v[t], 6);
  const __m256i q= _mm256_blend_epi32(_mm256_permutevar8x32_epi32(p, up), carry, 0x01);
  int bits= _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(p, q)));
  if(rem < 8) bits&= (1 << rem) - 1;
  if(bits) return true;
  carry= _mm256_permutevar8x32_epi32(p, last);
 }
 return false;
}
// 並べた鍵 k[0, m) の上の 26 bit が等しい並びごとに、d の同じ範囲を挿入ソートで値の順に直す。
inline void fix_ties(const u32* k, int m, T* d) {
 for(int i= 0; i < m;) {
  int j= i + 1;
  while(j < m && (k[j] ^ k[i]) < 64) ++j;
  for(int x= i + 1; x < j; ++x) {
   const T y= d[x];
   int z= x;
   for(; z > i && y < d[z - 1]; --z) d[z]= d[z - 1];
   d[z]= y;
  }
  i= j;
 }
}
// 塊 s[0, m) (2 <= m <= 64) を並べて d[0, m) に書く。fix は鍵が桁より下の bit を全部は持たないとき。
[[gnu::always_inline]] inline void sort_bucket(const T* s, int m, const KeyParams& kp, bool fix, T* d) {
 alignas(32) u32 k[64];
 __m256i v[8];
 const __m256i pad= _mm256_set1_epi32(-1);
 int nv;
 if(m <= 8) {
  nv= 1, v[0]= net32::sort8(make_keys(s, m, 0, kp));
 } else if(m <= 16) {
  nv= 2, v[0]= make_keys(s, m, 0, kp), v[1]= make_keys(s, m, 1, kp);
  net32::sort16(v[0], v[1]);
 } else if(m <= 32) {
  nv= 4;
  for(int t= 0; t < 4; ++t) v[t]= 8 * t < m ? make_keys(s, m, t, kp) : pad;
  net32::sort32(v[0], v[1], v[2], v[3]);
 } else {
  nv= 8;
  for(int t= 0; t < 8; ++t) v[t]= 8 * t < m ? make_keys(s, m, t, kp) : pad;
  net32::sort64(v);
 }
 for(int t= 0; t < nv; ++t) _mm256_store_si256((__m256i*)(k + 8 * t), v[t]);
 for(int i= 0; i < m; ++i) d[i]= s[k[i] & 63];
 if(fix && has_tie(v, nv, m)) fix_ties(k, m, d);
}
// src[0, n) を並べた列を dst[0, n) に書く (src は書き換えない)。self-sort-i64 の msd16_net32 の sort と同じ形で、a の代わりに dst に書く。
// 上の桁で 1 回振り分けてから、塊ごとに u32 の鍵のネットワークで並べる。
inline void sort_to(const T* src, size_t n, T* dst) {
 if(n <= 32) return small_sort_to(src, dst, n);
 u64 o= 0, e= ~0ull, mn= ~0ull, mx= 0;
 for(size_t i= 0; i < n; ++i) {
  const u64 k= key(src[i]);
  o|= k, e&= k, mn= std::min(mn, k), mx= std::max(mx, k);
 }
 const u64 diff= o ^ e;
 if(!diff) return (void)std::memcpy(dst, src, n * sizeof(T));  // 全要素が同じ値
 int hi= 63 - __builtin_clzll(diff), lo= __builtin_ctzll(diff);
 u64 base= 0;
 if(const int wm= 64 - __builtin_clzll(mx - mn); wm < hi - lo + 1) lo= 0, hi= wm - 1, base= mn;
 int db= 1;
 while(db < 16 && (n >> db) > 16) ++db;
 db= std::min(db, hi - lo + 1);
 const int sh= hi - db + 1, r= sh - lo, rk= std::min(r, 26);
 const u64 mk= (u64(1) << db) - 1;
 std::vector<u32> pos(size_t(1) << db);
 for(size_t i= 0; i < n; ++i) ++pos[(key(src[i]) - base) >> sh & mk];
 for(size_t d= 0, s= 0; d < pos.size(); ++d) {
  const u32 t= pos[d];
  pos[d]= u32(s), s+= t;
 }
 auto buf= alloc_huge(n + 64);  // 塊を 8 個ずつ読むので、最後の塊の後ろを 63 個まで読む
 T* b= buf.get();
 for(size_t i= 0; i < n; ++i) {
  const T x= src[i];
  b[pos[(key(x) - base) >> sh & mk]++]= x;
 }
 if(!r) {  // 桁より下の bit が全要素で同じなので、振り分けた順で並んでいる
  std::memcpy(dst, b, n * sizeof(T));
  return;
 }
 // 振り分けたあとの pos[d] は塊 d の終わり。
 const KeyParams kp{SIGNED ? _mm256_set1_epi64x((long long)(1ull << 63)) : _mm256_setzero_si256(), _mm256_set1_epi64x((long long)base),
                    _mm256_set1_epi64x((long long)((u64(1) << rk) - 1)), _mm_cvtsi32_si128(sh - rk)};
 const bool fix= r > rk;
 for(size_t d= 0, off= 0; d < pos.size(); ++d) {
  const size_t end= pos[d], m= end - off;
  if(m <= 1) {
   if(m) dst[off]= b[off];
  } else if(m <= 64) {
   sort_bucket(b + off, int(m), kp, fix, dst + off);
  } else {
   int log= 0;
   for(size_t t= m; t >>= 1;) ++log;
   rec(b + off, dst + off, m, 2 * log, false);
  }
  off= end;
 }
}
// clang は二分探索で比較から選ぶところを分岐に直すので、__builtin_unpredictable で cmov のまま残させる。GCC はそのままで cmov にする。
#ifdef __clang__
#define SORT_UNPREDICTABLE(c) __builtin_unpredictable(c)
#else
#define SORT_UNPREDICTABLE(c) (c)
#endif
// 並んだ列 s[0, k) (k >= 1) で x 以上の最初の位置。x が列にあれば、その位置になる。分岐しない二分探索。
inline size_t lower_bound_bl(const T* s, size_t k, T x) {
 const T* p= s;
 while(k > 1) {
  const size_t half= k >> 1;
  p+= SORT_UNPREDICTABLE(p[half] < x) ? half : 0;
  k-= half;
 }
 return size_t(p - s) + (*p < x);
}
// 256 個以下: a を写して std::sort と std::unique で xs を作り、各要素を分岐しない二分探索で引く。
inline std::vector<T> compress_small(std::vector<T>& a) {
 std::vector<T> xs(a);
 std::sort(xs.begin(), xs.end());
 xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
 for(auto& x: a) x= T(lower_bound_bl(xs.data(), xs.size(), x));
 return xs;
}
inline std::vector<T> compress(std::vector<T>& a) {
 const size_t n= a.size();
 if(n <= 256) return compress_small(a);
 auto ob= alloc_huge(n + 64);
 T* s= ob.get();
 sort_to(a.data(), n, s);
 const size_t k= std::unique(s, s + n) - s;
 std::vector<T> xs(s, s + k);
 for(size_t i= 0; i < n; ++i) a[i]= T(lower_bound_bl(s, k, a[i]));
 return xs;
}
}  // namespace compress_sort_lb
inline std::vector<long long> run(std::vector<long long>& a) { return compress_sort_lb::compress(a); }
