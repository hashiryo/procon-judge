#pragma once
// msd16_net_nm の、塊ごとに呼ぶ関数とその中のネットワークの部品を always_inline にした版。インライン化をコンパイラに任せると、
// 外に出る関数が書き換えのたびに変わり、self-sort-compress-u32 では同じ回の中でも 1 割から 2 割の差が出た。ここでは、
// 外に出していた分を展開させると速くなるかを見る。ほかは msd16_net_nm と同じ。以下は msd16_net_nm の説明。
// msd16_net の塊を並べるところで、vpmaskmovd の読み書きをやめた版。作業用の配列の後ろに 8 個の余白を取り、塊の半端なベクタも
// ふつうに 8 個読んでから、塊より後ろの lane を最大値で埋める。書き戻しも 8 個ずつふつうに書き、塊より後ろにはみ出した分は、後ろの
// 塊を並べるときに上書きされる (塊は前から順に並べる)。a の終わりを越える塊だけは vpmaskmovd で書く。AMD では vpmaskmovd の書き込み
// が重いと言われるので、どれだけ効くかを見る。ほかは msd16_net と同じ。以下は msd16_net の説明。
// 上の 16 bit で 1 回振り分けてから、塊ごとにソーティングネットワークで並べる版 (self-sort-u64 の msd16_net を u32 にしたもの)。
// 一様な 10^6 個なら塊は 65536 個で平均 15 個になり、全体をなめる振り分けは 1 回で済む (LSD は 3 回)。ネットワーク、vpmaskmovd での
// 読み書き、64 個を超えた塊の分割は qsort_avx2_oop_all の部品を使う。u32 は 8 lane なので 64 個までのネットワークが安い。
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
namespace sort_msd16_net_nm_ai {
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
// 表の m 番目: bit i が立っている lane (軸より大きい) を後ろに、立っていない lane を前に集める vpermd の添字を
// 3 bit ずつ詰めたもの。
struct PermTable {
 u32 t[256];
 constexpr PermTable(): t() {
  for(int m= 0; m < 256; ++m) {
   u32 packed= 0;
   int k= 0;
   for(int i= 0; i < 8; ++i)
    if(!(m >> i & 1)) packed|= u32(i) << (3 * k++);
   for(int i= 0; i < 8; ++i)
    if(m >> i & 1) packed|= u32(i) << (3 * k++);
   t[m]= packed;
  }
 }
};
inline constexpr PermTable PERM{};
// x[0, n) を、p 以下を y の前から、p より大きいものを y の後ろから詰めて書き、前に書いた個数を返す。8 個ずつ読んで軸と比べ、
// movemask の 8 bit で表を引いて vpermd で並べ替え、同じベクタを y の前の書き込み位置と後ろの書き込み位置の両方に書く。
// 読む位置は先頭から順に進むだけなので、その場で分けるときの「左右のどちらから読むか」の選び分けが要らない。
inline size_t partition(const u32* x, u32* y, size_t n, u32 p) {
 const __m256i sign= _mm256_set1_epi32(int(0x80000000u));
 const __m256i pv= _mm256_set1_epi32(int(p ^ 0x80000000u));
 const __m256i shifts= _mm256_setr_epi32(0, 3, 6, 9, 12, 15, 18, 21);
 // [l, r) がまだ書いていない範囲で、幅は読んでいない個数と同じ。前の書き込み [l, l + 8) と後ろの書き込み [r - 8, r) が
 // 重なると、後ろの書き込みが前に置いた値を上書きするので、16 個以上残っている間だけベクタで進め、残りは 1 個ずつ置く。
 size_t l= 0, r= n, i= 0;
 for(; i + 16 <= n; i+= 8) {
  const __m256i v= _mm256_loadu_si256((const __m256i*)(x + i));
  const int m= _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(_mm256_xor_si256(v, sign), pv)));
  const __m256i w= _mm256_permutevar8x32_epi32(v, _mm256_srlv_epi32(_mm256_set1_epi32(int(PERM.t[m])), shifts));
  const int nr= __builtin_popcount(m);
  _mm256_storeu_si256((__m256i*)(y + l), w);
  _mm256_storeu_si256((__m256i*)(y + r - 8), w);
  l+= 8 - nr, r-= nr;
 }
 for(; i < n; ++i) {
  if(x[i] <= p) y[l++]= x[i];
  else y[--r]= x[i];
 }
 return l;
}
// bitonic のソーティングネットワーク。stepD<MASK> は距離 D の lane どうしを比べ、MASK の bit が立つ lane に大きい方を置く。
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
// a[0, n) (n <= 64) を、8 個ずつのベクタに最大値で埋めて読み、8 / 16 / 32 / 64 個のネットワークで並べて書き戻す。端の半端なベクタは
// vpmaskmovd で読み書きし、配列の外には触れない (作業用の配列と memcpy を使わない)。rem はそのベクタから先に残っている個数。
inline __m256i load_pad(const u32* a, int rem) {
 if(rem >= 8) return _mm256_loadu_si256((const __m256i*)a);
 const __m256i m= _mm256_cmpgt_epi32(_mm256_set1_epi32(rem), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
 return _mm256_blendv_epi8(_mm256_set1_epi32(-1), _mm256_maskload_epi32((const int*)a, m), m);
}
inline void store_part(u32* a, int rem, __m256i v) {
 if(rem >= 8) return _mm256_storeu_si256((__m256i*)a, v);
 const __m256i m= _mm256_cmpgt_epi32(_mm256_set1_epi32(rem), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
 _mm256_maskstore_epi32((int*)a, m, v);
}
// src[0, n) (n <= 64) を並べて dst[0, n) に書く (src と dst は同じでもよい)。
inline void small_sort_to(const u32* src, u32* dst, size_t n) {
 if(n <= 1) {
  if(n) dst[0]= src[0];
  return;
 }
 const int r= int(n);
 if(n <= 8) {
  store_part(dst, r, sort8(load_pad(src, r)));
 } else if(n <= 16) {
  __m256i x= load_pad(src, 8), y= load_pad(src + 8, r - 8);
  sort16(x, y);
  store_part(dst, 8, x), store_part(dst + 8, r - 8, y);
 } else if(n <= 32) {
  __m256i x= load_pad(src, 8), y= load_pad(src + 8, 8), z= load_pad(src + 16, r - 16), w= load_pad(src + 24, r - 24);
  sort32(x, y, z, w);
  store_part(dst, 8, x), store_part(dst + 8, 8, y), store_part(dst + 16, r - 16, z), store_part(dst + 24, r - 24, w);
 } else {
  __m256i v[8];
  for(int j= 0; j < 8; ++j) v[j]= load_pad(src + 8 * j, r - 8 * j);
  sort64(v);
  for(int j= 0; j < 8; ++j) store_part(dst + 8 * j, r - 8 * j, v[j]);
 }
}
inline void small_sort(u32* a, size_t n) { small_sort_to(a, a, n); }
// src[0, m) (2 <= m <= 64) を並べて dst[0, m) に書く。src は塊の後ろを 7 個まで読んでよい (並べる値には使わない)。wide なら dst も
// 塊の後ろを 7 個まで書いてよく、そうでなければ半端なベクタは vpmaskmovd で書く。
[[gnu::always_inline]] inline __m256i load_nm(const u32* a, int rem) {
 const __m256i v= _mm256_loadu_si256((const __m256i*)a);
 if(rem >= 8) return v;
 return _mm256_blendv_epi8(_mm256_set1_epi32(-1), v, _mm256_cmpgt_epi32(_mm256_set1_epi32(rem), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7)));
}
[[gnu::always_inline]] inline void store_nm(u32* a, int rem, __m256i v, bool wide) {
 if(wide || rem >= 8) return _mm256_storeu_si256((__m256i*)a, v);
 store_part(a, rem, v);
}
[[gnu::always_inline]] inline void small_sort_nm(const u32* src, u32* dst, int m, bool wide) {
 const __m256i pad= _mm256_set1_epi32(-1);
 if(m <= 8) {
  store_nm(dst, m, sort8(load_nm(src, m)), wide);
 } else if(m <= 16) {
  __m256i x= load_nm(src, 8), y= load_nm(src + 8, m - 8);
  sort16(x, y);
  store_nm(dst, 8, x, wide), store_nm(dst + 8, m - 8, y, wide);
 } else if(m <= 32) {
  __m256i v[4];
  for(int t= 0; t < 4; ++t) v[t]= 8 * t < m ? load_nm(src + 8 * t, m - 8 * t) : pad;
  sort32(v[0], v[1], v[2], v[3]);
  for(int t= 0; t < 4 && 8 * t < m; ++t) store_nm(dst + 8 * t, m - 8 * t, v[t], wide);
 } else {
  __m256i v[8];
  for(int t= 0; t < 8; ++t) v[t]= 8 * t < m ? load_nm(src + 8 * t, m - 8 * t) : pad;
  sort64(v);
  for(int t= 0; t < 8 && 8 * t < m; ++t) store_nm(dst + 8 * t, m - 8 * t, v[t], wide);
 }
}
inline u32 median3(u32 a, u32 b, u32 c) { return std::max(std::min(a, b), std::min(std::max(a, b), c)); }
// 8192 個以上の塊は、等間隔に取った 64 個の標本を 64 個のネットワークで並べ、中央の値を軸にする (偏りが減り、分割の段数が減る)。
// 128 個以上は 9 点、それ未満は 3 点の中央値。
inline u32 choose_pivot(const u32* a, size_t n) {
 if(n >= 8192) {
  alignas(32) u32 s[64];
  const size_t step= n / 64;
  for(int i= 0; i < 64; ++i) s[i]= a[i * step + step / 2];
  __m256i v[8];
  for(int j= 0; j < 8; ++j) v[j]= _mm256_load_si256((const __m256i*)(s + 8 * j));
  sort64(v);
  return u32(_mm256_extract_epi32(v[4], 0));
 }
 if(n < 128) return median3(a[0], a[n / 2], a[n - 1]);
 const size_t s= n / 8;
 return median3(median3(a[0], a[s], a[2 * s]), median3(a[3 * s], a[4 * s], a[5 * s]), median3(a[6 * s], a[7 * s], a[n - 1]));
}
// x[0, n) を並べる。y は同じ長さの作業用の領域で、分割のたびに x と y を入れ替える。x_is_a は x が元の配列 a の側かで、
// 並べ終えた値は a の側に置く (a の側でなければ最後に y へ写す)。
inline void rec(u32* x, u32* y, size_t n, int depth, bool x_is_a) {
 while(n > 64) {
  if(depth-- == 0) {
   std::sort(x, x + n);
   if(!x_is_a) std::memcpy(y, x, n * sizeof(u32));
   return;
  }
  const u32 p= choose_pivot(x, n);
  size_t k= partition(x, y, n, p);
  std::swap(x, y), x_is_a= !x_is_a;
  if(k == n) {
   // 全要素が p 以下なので p は最大値。p と等しい要素 (並べ終わり) を後ろに分け、前だけを続ける。
   if(p == 0) {  // 全要素が 0
    if(!x_is_a) std::memcpy(y, x, n * sizeof(u32));
    return;
   }
   k= partition(x, y, n, p - 1);
   std::swap(x, y), x_is_a= !x_is_a;
   if(!x_is_a) std::memcpy(y + k, x + k, (n - k) * sizeof(u32));
   n= k;
   continue;
  }
  // 小さい側を再帰で、大きい側をループで並べる (スタックの深さを log n に抑える)。
  if(k < n - k) rec(x, y, k, depth, x_is_a), x+= k, y+= k, n-= k;
  else rec(x + k, y + k, n - k, depth, x_is_a), n= k;
 }
 small_sort(x, n);
 if(!x_is_a) std::memcpy(y, x, n * sizeof(u32));
}
// 上の桁で 1 回振り分けてから、塊ごとにネットワークで並べる。最初の走査で全要素の bit or と bit and を取り、要素によって違う bit の
// 上から db bit を桁にする。db は塊の平均が 16 個以下になる最小の幅 (16 bit まで)。a から作業用の配列 (huge page) へ振り分け、
// 塊ごとに作業用の配列から読んで並べ、a の同じ位置に書く。64 個以下の塊はネットワーク、それより大きい塊は qsort_avx2_oop_all と
// 同じ分割に任せる。
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 64) return small_sort(v.data(), n);
 u32* a= v.data();
 u32 o= 0, e= ~0u;
 for(size_t i= 0; i < n; ++i) o|= a[i], e&= a[i];
 const u32 diff= o ^ e;
 if(!diff) return;  // 全要素が同じ値
 const int hi= 31 - __builtin_clz(diff), lo= __builtin_ctz(diff);
 int db= 1;
 while(db < 16 && (n >> db) > 16) ++db;
 db= std::min(db, hi - lo + 1);
 const int sh= hi - db + 1;
 const u32 mk= (1u << db) - 1;
 std::vector<u32> cnt(size_t(1) << db), head(size_t(1) << db);
 for(size_t i= 0; i < n; ++i) ++cnt[a[i] >> sh & mk];
 for(size_t d= 0, s= 0; d < cnt.size(); ++d) head[d]= u32(s), s+= cnt[d];
 auto buf= alloc_huge(n + 8);  // 塊の半端なベクタを 8 個ずつ読むので、最後の塊の後ろを 7 個まで読む
 u32* b= buf.get();
 {
  std::vector<u32> pos(head);
  for(size_t i= 0; i < n; ++i) {
   const u32 x= a[i];
   b[pos[x >> sh & mk]++]= x;
  }
 }
 for(size_t d= 0; d < cnt.size(); ++d) {
  const size_t m= cnt[d], off= head[d];
  if(m <= 1) {
   if(m) a[off]= b[off];
  } else if(m <= 64) {
   small_sort_nm(b + off, a + off, int(m), off + ((m + 7) & ~size_t(7)) <= n);
  } else {
   int log= 0;
   for(size_t t= m; t >>= 1;) ++log;
   rec(b + off, a + off, m, 2 * log, false);
  }
 }
}
}  // namespace sort_msd16_net_nm_ai
inline void run(std::vector<unsigned>& a) { sort_msd16_net_nm_ai::sort(a); }
