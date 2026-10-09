#pragma once
// pack_msd16_net32 の、塊ごとに呼ぶ関数とその中のネットワークの部品を always_inline にした版。インライン化をコンパイラに任せると、
// 外に出る関数が書き換えのたびに変わり、self-sort-compress-u32 では同じ回の中でも 1 割から 2 割の差が出た。ここでは、
// 外に出していた分を展開させると速くなるかを見る。ほかは pack_msd16_net32 と同じ。以下は pack_msd16_net32 の説明。
// pack_msd16_net の塊の中を、u64 のネットワークでなく u32 のネットワークで並べる版。値と添字を詰めた u64 を値の上の桁で振り分けたあと、
// 塊の中では桁とそれより上の bit がそろっているので、桁より下の bit (26 bit 以下) を上に、塊の中の位置を下の 6 bit に詰めた u32 を鍵にして
// 並べられる。振り分けは元の順を保つので、塊の中の位置の順は添字の順と同じで、等しい値は添字の小さい順に並ぶ。u32 は 8 lane で min と
// max も 1 命令なので、4 lane の u64 を比較と blend で並べるより軽い。鍵は詰めた u64 を 8 個ずつ読んでレジスタの中で作り、並べたあと
// 鍵の下の 6 bit で詰めた u64 を引いて、添字を返り値に書く。桁は値の bit or と bit and から、要素によって違う bit の上から、塊の平均が
// 16 個以下になる幅 (6 bit から 16 bit まで) を取る。桁より下の bit が全要素で同じなら、振り分け 1 回で添字を直接書く。64 個を超える
// 塊は、詰めた u64 が並んでいなければ std::sort で並べる。ネットワークの部品は self-sort-u32 の msd16_net と同じ。作業用の配列は
// huge page にする。
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
#include <memory>
#include <numeric>
#include <vector>
namespace argsort_pack_msd16_net32_ai {
using u32= unsigned;
using u64= unsigned long long;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
template <class T> inline std::unique_ptr<T, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(T) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<T, FreeDeleter>(static_cast<T*>(p));
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
// 塊 s[0, m) の 8t 番目からの 8 個の鍵を作る。詰めた u64 を 4 個ずつ 2 回読み、上位 32 bit (値) を元の順に集めてから、値の bit
// [lo, lo + r) を上に、塊の中の位置を下の 6 bit に詰める。m 個より後ろの lane は最大値で埋める (読むのは s[0, 8t + 8) で、塊の後ろの
// 要素を読んでも鍵には使わない)。
[[gnu::always_inline]] inline __m256i make_keys(const u64* s, int m, int t, __m128i lo, __m256i rmask) {
 const int base= 8 * t;
 const __m256 v0= _mm256_castsi256_ps(_mm256_loadu_si256((const __m256i*)(s + base)));
 const __m256 v1= _mm256_castsi256_ps(_mm256_loadu_si256((const __m256i*)(s + base + 4)));
 // shuffle_ps で 128 bit の中ごとに奇数番目の 32 bit を集めると (x0 x1 x4 x5 | x2 x3 x6 x7) になるので、64 bit 単位で並べ直す。
 const __m256i x= _mm256_permute4x64_epi64(_mm256_castps_si256(_mm256_shuffle_ps(v0, v1, 0xDD)), 0xD8);
 const __m256i iota= _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
 const __m256i k= _mm256_or_si256(_mm256_slli_epi32(_mm256_and_si256(_mm256_srl_epi32(x, lo), rmask), 6), _mm256_add_epi32(iota, _mm256_set1_epi32(base)));
 if(m - base >= 8) return k;
 return _mm256_blendv_epi8(_mm256_set1_epi32(-1), k, _mm256_cmpgt_epi32(_mm256_set1_epi32(m - base), iota));
}
// 塊 s[0, m) (2 <= m <= 64) を値の順に並べ、添字を d[0, m) に書く。
[[gnu::always_inline]] inline void sort_bucket(const u64* s, int m, __m128i lo, __m256i rmask, u32* d) {
 alignas(32) u32 k[64];
 const __m256i pad= _mm256_set1_epi32(-1);
 if(m <= 8) {
  _mm256_store_si256((__m256i*)k, sort8(make_keys(s, m, 0, lo, rmask)));
 } else if(m <= 16) {
  __m256i x= make_keys(s, m, 0, lo, rmask), y= make_keys(s, m, 1, lo, rmask);
  sort16(x, y);
  _mm256_store_si256((__m256i*)k, x), _mm256_store_si256((__m256i*)(k + 8), y);
 } else if(m <= 32) {
  __m256i v[4];
  for(int t= 0; t < 4; ++t) v[t]= 8 * t < m ? make_keys(s, m, t, lo, rmask) : pad;
  sort32(v[0], v[1], v[2], v[3]);
  for(int t= 0; t < 4; ++t) _mm256_store_si256((__m256i*)(k + 8 * t), v[t]);
 } else {
  __m256i v[8];
  for(int t= 0; t < 8; ++t) v[t]= 8 * t < m ? make_keys(s, m, t, lo, rmask) : pad;
  sort64(v);
  for(int t= 0; t < 8; ++t) _mm256_store_si256((__m256i*)(k + 8 * t), v[t]);
 }
 for(int t= 0; t < m; ++t) d[t]= u32(s[k[t] & 63]);
}
inline std::vector<u32> argsort(const std::vector<u32>& a) {
 const size_t n= a.size();
 std::vector<u32> p(n);
 const u32* key= a.data();
 if(n <= 64) {
  u64 b[64]= {};
  for(size_t i= 0; i < n; ++i) b[i]= (u64(key[i]) << 32) | i;
  std::sort(b, b + n);
  for(size_t i= 0; i < n; ++i) p[i]= u32(b[i]);
  return p;
 }
 u32 o= 0, e= ~0u;
 for(size_t i= 0; i < n; ++i) o|= key[i], e&= key[i];
 const u32 diff= o ^ e;
 if(!diff) {  // 全要素が同じ値
  std::iota(p.begin(), p.end(), 0u);
  return p;
 }
 const int hi= 31 - __builtin_clz(diff), lo= __builtin_ctz(diff), w= hi - lo + 1;
 // 6 bit 以上にするのは、桁より下の bit を 26 bit 以下にして、塊の中の位置 (6 bit) と u32 に詰めるため。
 int db= 6;
 while(db < 16 && (n >> db) > 16) ++db;
 db= std::min(db, w);
 const int sh= hi - db + 1, r= sh - lo;
 const u32 mk= (1u << db) - 1;
 std::vector<u32> pos(size_t(1) << db);
 for(size_t i= 0; i < n; ++i) ++pos[key[i] >> sh & mk];
 for(size_t d= 0, s= 0; d < pos.size(); ++d) {
  const u32 t= pos[d];
  pos[d]= u32(s), s+= t;
 }
 if(!r) {  // 桁より下の bit が全要素で同じ: 振り分け 1 回で添字を直接書く
  for(size_t i= 0; i < n; ++i) p[pos[key[i] >> sh & mk]++]= u32(i);
  return p;
 }
 auto bb= alloc_huge<u64>(n + 64);  // 塊を 8 個ずつ読むので、最後の塊の後ろを 63 個まで読む
 u64* b= bb.get();
 for(size_t i= 0; i < n; ++i) {
  const u32 x= key[i];
  b[pos[x >> sh & mk]++]= (u64(x) << 32) | i;
 }
 // 振り分けたあとの pos[d] は塊 d の終わり。
 const __m128i lov= _mm_cvtsi32_si128(lo);
 const __m256i rmask= _mm256_set1_epi32(int((1u << r) - 1));
 u32* q= p.data();
 for(size_t d= 0, off= 0; d < pos.size(); ++d) {
  const size_t end= pos[d], m= end - off;
  if(m <= 1) {
   if(m) q[off]= u32(b[off]);
  } else if(m <= 64) {
   sort_bucket(b + off, int(m), lov, rmask, q + off);
  } else {
   if(!std::is_sorted(b + off, b + end)) std::sort(b + off, b + end);
   for(size_t t= off; t < end; ++t) q[t]= u32(b[t]);
  }
  off= end;
 }
 return p;
}
}  // namespace argsort_pack_msd16_net32_ai
inline std::vector<unsigned> run(const std::vector<unsigned>& a) { return argsort_pack_msd16_net32_ai::argsort(a); }
