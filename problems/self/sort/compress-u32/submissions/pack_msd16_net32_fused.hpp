#pragma once
// pack_msd16_net32 で、塊を並べたその場で順位を数えて書く版。並べた列を書いてから読み直す分を省く。値と添字を詰めた u64 を値の
// 上の桁で振り分け、塊の中を u32 の鍵 (桁より下の bit を上に、塊の中の位置を下の 6 bit に詰めたもの) の AVX2 のネットワークで
// 並べたら、並べた順に鍵の上の bit を比べて順位を数え、a の添字の位置に書く。塊は値の順に並んでいて、違う塊の値は必ず違うので、
// 順位は塊をまたいで数え続ければよい。桁より下の bit が全要素で同じなら、空でない塊ごとに値が 1 つなので、振り分けずに、塊の番号から
// 順位を引く表で各要素を書き換える。64 個を超える塊は、並んでいなければ std::sort で並べてから走査する。作業用の配列は huge page に
// する。ほかの説明は self-sort-argsort-u32 の pack_msd16_net32 にある。
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
#include <vector>
namespace compress_pack_msd16_net32_fused {
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
inline void merge16(__m256i& a, __m256i& b) {
 __m256i l= vmin(a, b), h= vmax(a, b);
 a= merge8(l), b= merge8(h);
}
inline void sort16(__m256i& a, __m256i& b) {
 a= sort8(a), b= reverse8(sort8(b));
 merge16(a, b);
}
inline void sort32(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 sort16(a, b), sort16(c, d);
 // 後ろの 16 個を逆順にして (reverse8(d), reverse8(c)) とつなぐと 32 個が bitonic になる。
 __m256i rc= reverse8(d), rd= reverse8(c);
 __m256i l0= vmin(a, rc), h0= vmax(a, rc), l1= vmin(b, rd), h1= vmax(b, rd);
 merge16(l0, l1), merge16(h0, h1);
 a= l0, b= l1, c= h0, d= h1;
}
// a, b, c, d を並べた 32 個が bitonic なら昇順にする。
inline void merge32(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 __m256i l0= vmin(a, c), h0= vmax(a, c), l1= vmin(b, d), h1= vmax(b, d);
 merge16(l0, l1), merge16(h0, h1);
 a= l0, b= l1, c= h0, d= h1;
}
// 8 本のベクタの 64 個を昇順にする。前後の 32 個を並べ、後ろを逆順にしてつないだ bitonic な 64 個を merge する。
inline void sort64(__m256i* v) {
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
inline __m256i make_keys(const u64* s, int m, int t, __m128i lo, __m256i rmask) {
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
// 塊 s[0, m) (2 <= m <= 64) の鍵を並べて k[0, m) に書く。
inline void sort_keys(const u64* s, int m, __m128i lo, __m256i rmask, u32* k) {
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
}
// 値の順に並んだ詰めた u64 の列 s[0, n) (n >= 1) を先頭から走査する。値が変わるたびに順位を 1 増やして xs に足し、a の添字の
// 位置に順位を書く。
inline std::vector<u32> scan(const u64* s, size_t n, u32* a) {
 std::vector<u32> xs;
 xs.reserve(n);
 u32 prev= u32(s[0] >> 32), r= 0;
 xs.push_back(prev);
 for(size_t i= 0; i < n; ++i) {
  const u64 x= s[i];
  const u32 v= u32(x >> 32);
  if(v != prev) xs.push_back(v), prev= v, ++r;
  a[u32(x)]= r;
 }
 return xs;
}
// 全要素が同じ値 (n >= 1)。
inline std::vector<u32> all_same(u32* a, size_t n) {
 std::vector<u32> xs(1, a[0]);
 std::fill(a, a + n, 0u);
 return xs;
}
// 塊 s[0, m) (2 <= m <= 64) を鍵のネットワークで並べ、並べた順に順位を数えて a の添字の位置に書き、新しい値を xs に足す。rk は
// それまでの値の種類数 (次に付ける順位)。塊の中では桁とそれより上の bit、lo より下の bit がそろっているので、鍵の上の 26 bit 以下
// (値の bit [lo, sh)) が等しいことと値が等しいことは同じ。
inline void rank_bucket(const u64* s, int m, __m128i lo, __m256i rmask, u32* a, std::vector<u32>& xs, u32& rk) {
 alignas(32) u32 k[64];
 sort_keys(s, m, lo, rmask, k);
 u32 prev= ~0u;  // 鍵の上の bit は 26 bit 以下なので、塊の最初の要素は必ず新しい値になる
 for(int t= 0; t < m; ++t) {
  const u32 v= k[t] >> 6;
  const u64 x= s[k[t] & 63];
  if(v != prev) xs.push_back(u32(x >> 32)), prev= v, ++rk;
  a[u32(x)]= rk - 1;
 }
}
inline std::vector<u32> compress(std::vector<u32>& a) {
 const size_t n= a.size();
 u32* key= a.data();
 if(!n) return {};
 if(n <= 64) {
  u64 b[64]= {};
  for(size_t i= 0; i < n; ++i) b[i]= (u64(key[i]) << 32) | i;
  std::sort(b, b + n);
  return scan(b, n, key);
 }
 u32 o= 0, e= ~0u;
 for(size_t i= 0; i < n; ++i) o|= key[i], e&= key[i];
 const u32 diff= o ^ e;
 if(!diff) return all_same(key, n);
 const int hi= 31 - __builtin_clz(diff), lo= __builtin_ctz(diff), w= hi - lo + 1;
 // 6 bit 以上にするのは、桁より下の bit を 26 bit 以下にして、塊の中の位置 (6 bit) と u32 に詰めるため。
 int db= 6;
 while(db < 16 && (n >> db) > 16) ++db;
 db= std::min(db, w);
 const int sh= hi - db + 1, r= sh - lo;
 const u32 mk= (1u << db) - 1;
 std::vector<u32> pos(size_t(1) << db);
 for(size_t i= 0; i < n; ++i) ++pos[key[i] >> sh & mk];
 if(!r) {
  // 桁が値を決めるので、空でない塊ごとに値が 1 つある。値は、桁の外の bit (全要素で同じ) に塊の番号を入れたもの。
  std::vector<u32> xs;
  xs.reserve(std::min(n, pos.size()));
  const u32 base= e & ~(mk << sh);
  u32 rk= 0;
  for(size_t d= 0; d < pos.size(); ++d) {
   const u32 c= pos[d];
   pos[d]= rk;
   if(c) xs.push_back(base | u32(d) << sh), ++rk;
  }
  for(size_t i= 0; i < n; ++i) key[i]= pos[key[i] >> sh & mk];
  return xs;
 }
 for(size_t d= 0, s= 0; d < pos.size(); ++d) {
  const u32 t= pos[d];
  pos[d]= u32(s), s+= t;
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
 std::vector<u32> xs;
 xs.reserve(n);
 u32 rk= 0;
 for(size_t d= 0, off= 0; d < pos.size(); ++d) {
  const size_t end= pos[d], m= end - off;
  if(m == 1) {
   const u64 x= b[off];
   xs.push_back(u32(x >> 32));
   key[u32(x)]= rk++;
  } else if(m >= 2 && m <= 64) {
   rank_bucket(b + off, int(m), lov, rmask, key, xs, rk);
  } else if(m > 64) {
   if(!std::is_sorted(b + off, b + end)) std::sort(b + off, b + end);
   u32 prev= u32(b[off] >> 32);
   xs.push_back(prev), ++rk;
   for(size_t t= off; t < end; ++t) {
    const u64 x= b[t];
    const u32 v= u32(x >> 32);
    if(v != prev) xs.push_back(v), prev= v, ++rk;
    key[u32(x)]= rk - 1;
   }
  }
  off= end;
 }
 return xs;
}
}  // namespace compress_pack_msd16_net32_fused
inline std::vector<unsigned> run(std::vector<unsigned>& a) { return compress_pack_msd16_net32_fused::compress(a); }
