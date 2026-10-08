#pragma once
// qsort_avx2_oop のネットワークに任せる大きさを 64 個 (4 lane × 16 本) に上げた版。self-sort-u32 では 32 個から 64 個にして
// 17% 縮んだ。分割の段が 1 段減る代わりに、葉のネットワークが重くなる。
// self-sort-u32 の qsort_avx2_oop_all を 64 bit (4 lane) にした版。作業用の配列 (huge page) へ分割するクイックソートで、4 個ずつ先頭から
// 読んで軸と比べ、movemask の 4 bit で表を引いて vpermd で軸以下を前、軸より大きいものを後ろに並べ、同じベクタを作業用の配列の前の
// 書き込み位置と後ろの書き込み位置の両方に書く。前と後ろの書き込みが重ならないよう、残りが 8 個以上の間だけベクタで進める。分割の
// たびに a と作業用の配列の役を入れ替える。AVX2 には 64 bit の符号なしの比較が無いので、最上位 bit を反転して符号付きの vpcmpgtq で
// 比べる。軸は 8192 個以上の塊なら 64 個の標本の中央値、128 個以上は 9 点、それ未満は 3 点の中央値。全要素が軸以下になったときは
// 軸が最大値なので、軸と等しい要素を後ろに分けて前だけを続ける。32 個以下の塊は、最大値で埋めた 8 本までのベクタに vpmaskmovq で
// 読み、bitonic のソーティングネットワークで並べて書き戻す。64 bit の min と max は AVX2 に無いので、比較の結果と blend で作る。
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
namespace sort_qsort_avx2_oop_b64 {
using T= unsigned long long;
constexpr bool SIGNED= false;
using u32= unsigned;
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
// 8 本のベクタの 32 個が bitonic なら昇順にする。
inline void merge32(__m256i* v) {
 minmax(v[0], v[4]), minmax(v[1], v[5]), minmax(v[2], v[6]), minmax(v[3], v[7]);
 merge16(v[0], v[1], v[2], v[3]), merge16(v[4], v[5], v[6], v[7]);
}
// 16 本のベクタの 64 個を昇順にする。前後の 32 個を並べ、後ろを逆順にしてつないだ bitonic な 64 個を merge する。
inline void sort64(__m256i* v) {
 sort32(v), sort32(v + 8);
 __m256i r[8];
 for(int i= 0; i < 8; ++i) r[i]= reverse4(v[15 - i]);
 for(int i= 0; i < 8; ++i) minmax(v[i], r[i]);
 merge32(v), merge32(r);
 for(int i= 0; i < 8; ++i) v[8 + i]= r[i];
}
// a[0, n) (n <= 64) を、4 個ずつのベクタに最大値で埋めて読み、4 / 8 / 16 / 32 / 64 個のネットワークで並べて書き戻す。端の半端な
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
inline void small_sort(T* a, size_t n) {
 if(n <= 1) return;
 const long long r= (long long)n;
 if(n <= 4) {
  store_part(a, r, sort4(load_pad(a, r)));
 } else if(n <= 8) {
  __m256i x= load_pad(a, 4), y= load_pad(a + 4, r - 4);
  sort8(x, y);
  store_part(a, 4, x), store_part(a + 4, r - 4, y);
 } else if(n <= 16) {
  __m256i x= load_pad(a, 4), y= load_pad(a + 4, 4), z= load_pad(a + 8, r - 8), w= load_pad(a + 12, r - 12);
  sort16(x, y, z, w);
  store_part(a, 4, x), store_part(a + 4, 4, y), store_part(a + 8, r - 8, z), store_part(a + 12, r - 12, w);
 } else if(n <= 32) {
  __m256i v[8];
  for(int j= 0; j < 8; ++j) v[j]= load_pad(a + 4 * j, r - 4 * j);
  sort32(v);
  for(int j= 0; j < 8; ++j) store_part(a + 4 * j, r - 4 * j, v[j]);
 } else {
  __m256i v[16];
  for(int j= 0; j < 16; ++j) v[j]= load_pad(a + 4 * j, r - 4 * j);
  sort64(v);
  for(int j= 0; j < 16; ++j) store_part(a + 4 * j, r - 4 * j, v[j]);
 }
}
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
 while(n > 64) {
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
inline void sort(std::vector<T>& v) {
 const size_t n= v.size();
 if(n <= 64) return small_sort(v.data(), n);
 auto buf= alloc_huge(n);
 int log= 0;
 for(size_t t= n; t >>= 1;) ++log;
 rec(v.data(), buf.get(), n, 2 * log, true);
}
}  // namespace sort_qsort_avx2_oop_b64
inline void run(std::vector<unsigned long long>& a) { sort_qsort_avx2_oop_b64::sort(a); }
