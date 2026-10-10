#pragma once
// block16_hp の表の作り方を、塊ごとの 1 回の走査にまとめたもの。a の塊の 16 個を AVX2 の 2 本に読み、a の写し、
// 左からの累積の最小 pre、右からの累積の最小 suf、塊の最小をその場で書く。累積は 128 bit の半分ごとに alignr で
// 1 個と 2 個ずらして min を取り、半分の境を越える分は端の値を配って min を取る (8 個で 7 命令)。2 本目には 1 本目の
// 末尾を配って min を取る。塊ごとの最小の sparse table の段も、sparse と同じく 8 個ずつ vpminud で作る。block16_hp
// では、a を写す走査と、累積を 1 個ずつの鎖で作る走査が別にあった。クエリは block16_hp と同じ分岐の形。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <memory>
#include <vector>
namespace rmq_block16_fused {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
using Buf= std::unique_ptr<u32[], FreeDeleter>;
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline Buf alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(u32) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return Buf(static_cast<u32*>(p));
}
// 8 個の左からの累積の最小。
inline __m256i prefix8(__m256i x, __m256i ones) {
 x= _mm256_min_epu32(x, _mm256_alignr_epi8(x, ones, 12));
 x= _mm256_min_epu32(x, _mm256_alignr_epi8(x, ones, 8));
 return _mm256_min_epu32(x, _mm256_permute2x128_si256(_mm256_shuffle_epi32(x, 0xff), ones, 0x02));
}
// 8 個の右からの累積の最小。
inline __m256i suffix8(__m256i x, __m256i ones) {
 x= _mm256_min_epu32(x, _mm256_alignr_epi8(ones, x, 4));
 x= _mm256_min_epu32(x, _mm256_alignr_epi8(ones, x, 8));
 return _mm256_min_epu32(x, _mm256_permute2x128_si256(_mm256_shuffle_epi32(x, 0x00), ones, 0x31));
}
struct Table {
 static constexpr int LB= 4, B= 1 << LB;
 int n, nb, lg;
 Buf buf;
 u32 *v, *pre, *suf, *st;
 // 塊 b の 16 個 (x0, x1) から、写し、pre、suf、塊の最小を書く。
 void block(int b, __m256i x0, __m256i x1) {
  const __m256i ones= _mm256_set1_epi32(-1), seven= _mm256_set1_epi32(7), zero= _mm256_setzero_si256();
  const size_t o= size_t(b) << LB;
  _mm256_store_si256((__m256i*)(v + o), x0), _mm256_store_si256((__m256i*)(v + o + 8), x1);
  const __m256i p0= prefix8(x0, ones), p1= _mm256_min_epu32(prefix8(x1, ones), _mm256_permutevar8x32_epi32(p0, seven));
  const __m256i s1= suffix8(x1, ones), s0= _mm256_min_epu32(suffix8(x0, ones), _mm256_permutevar8x32_epi32(s1, zero));
  _mm256_store_si256((__m256i*)(pre + o), p0), _mm256_store_si256((__m256i*)(pre + o + 8), p1);
  _mm256_store_si256((__m256i*)(suf + o), s0), _mm256_store_si256((__m256i*)(suf + o + 8), s1);
  st[b]= u32(_mm256_cvtsi256_si32(s0));
 }
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), buf(alloc_huge((size_t(nb) << LB) * 3 + size_t(lg + 1) * nb)) {
  const size_t m= size_t(nb) << LB;
  v= buf.get(), pre= v + m, suf= pre + m, st= suf + m;
  const u32* src= a.data();
  const int full= n >> LB;
  for(int b= 0; b < full; ++b) block(b, _mm256_loadu_si256((const __m256i*)(src + (size_t(b) << LB))), _mm256_loadu_si256((const __m256i*)(src + (size_t(b) << LB) + 8)));
  if(full < nb) {
   alignas(32) u32 t[B];
   std::fill(t, t + B, ~0u);
   std::copy(src + (size_t(full) << LB), src + n, t);
   block(full, _mm256_load_si256((const __m256i*)t), _mm256_load_si256((const __m256i*)(t + 8)));
  }
  for(int k= 0; k < lg; ++k) {
   const u32* s= st + size_t(k) * nb;
   u32* d= st + size_t(k + 1) * nb;
   const int h= 1 << k, len= nb - 2 * h + 1;
   int i= 0;
   for(; i + 8 <= len; i+= 8) _mm256_storeu_si256((__m256i*)(d + i), _mm256_min_epu32(_mm256_loadu_si256((const __m256i*)(s + i)), _mm256_loadu_si256((const __m256i*)(s + i + h))));
   for(; i < len; ++i) d[i]= std::min(s[i], s[i + h]);
  }
 }
 // 塊の [x, y) の最小。x < y。
 u32 blocks(int x, int y) const {
  const int k= std::bit_width(unsigned(y - x)) - 1;
  const u32* row= st + size_t(k) * nb;
  return std::min(row[x], row[y - (1 << k)]);
 }
 // 塊 b の lane [lo, hi] の最小。
 u32 inblock(int b, int lo, int hi) const {
  const __m256i* p= (const __m256i*)(v + (size_t(b) << LB));
  const __m256i vlo= _mm256_set1_epi32(lo), vhi= _mm256_set1_epi32(hi), eight= _mm256_set1_epi32(8);
  __m256i idx= _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7), m= _mm256_set1_epi32(-1);
  for(int t= 0; t < B / 8; ++t) {
   const __m256i out= _mm256_or_si256(_mm256_cmpgt_epi32(vlo, idx), _mm256_cmpgt_epi32(idx, vhi));
   m= _mm256_min_epu32(m, _mm256_or_si256(_mm256_load_si256(p + t), out));
   idx= _mm256_add_epi32(idx, eight);
  }
  __m128i h= _mm_min_epu32(_mm256_castsi256_si128(m), _mm256_extracti128_si256(m, 1));
  h= _mm_min_epu32(h, _mm_shuffle_epi32(h, 0x4e));
  h= _mm_min_epu32(h, _mm_shuffle_epi32(h, 0xb1));
  return u32(_mm_cvtsi128_si32(h));
 }
 u32 query(int l, int r) const {
  const int j= r - 1, bl= l >> LB, bj= j >> LB;
  if(bl == bj) return inblock(bl, l & (B - 1), j & (B - 1));
  u32 x= std::min(suf[l], pre[j]);
  if(bj - bl > 1) x= std::min(x, blocks(bl + 1, bj));
  return x;
 }
};
}
struct Solver {
 rmq_block16_fused::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
