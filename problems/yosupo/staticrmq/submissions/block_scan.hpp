#pragma once
// 32 個ずつの塊に分けた RMQ。塊の中で左から累積した最小 pre と右から累積した最小 suf を持ち、塊ごとの最小の列には
// 1 本の配列の sparse table を載せる。クエリの両端が別の塊なら、suf[l]、pre[r - 1]、間の塊の sparse table の min を
// 取る (間が無ければ前の 2 つ)。両端が同じ塊なら、その塊の 32 個を AVX2 の 4 本で読み、[l, r) の外の lane を最大値で
// 埋めて min を取る。a の写しは塊の境に揃えて 64 byte 境界に置き、末尾を最大値で埋める。表は a の写し、pre、suf が
// N 個ずつと、塊の sparse table が N / 32 * 14 段で、N = 5 * 10^5 で合わせて 7 MB ほど。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <memory>
#include <vector>
namespace rmq_block_scan {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
using Buf= std::unique_ptr<u32[], FreeDeleter>;
inline Buf alloc(size_t n) { return Buf((u32*)std::aligned_alloc(64, (n * sizeof(u32) + 63) & ~size_t(63))); }
struct Table {
 static constexpr int LB= 5, B= 1 << LB;
 int n, nb, lg;
 Buf v, pre, suf, st;
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), v(alloc(size_t(nb) << LB)), pre(alloc(size_t(nb) << LB)), suf(alloc(size_t(nb) << LB)), st(alloc(size_t(lg + 1) * nb)) {
  std::copy(a.begin(), a.end(), v.get());
  std::fill(v.get() + n, v.get() + (size_t(nb) << LB), ~0u);
  for(int b= 0; b < nb; ++b) {
   const u32* x= v.get() + (size_t(b) << LB);
   u32 *p= pre.get() + (size_t(b) << LB), *s= suf.get() + (size_t(b) << LB);
   u32 c= ~0u;
   for(int i= 0; i < B; ++i) p[i]= c= std::min(c, x[i]);
   st[b]= c;
   c= ~0u;
   for(int i= B; i--;) s[i]= c= std::min(c, x[i]);
  }
  for(int k= 0; k < lg; ++k) {
   const u32* s= st.get() + size_t(k) * nb;
   u32* d= st.get() + size_t(k + 1) * nb;
   for(int i= 0, len= nb - (2 << k) + 1; i < len; ++i) d[i]= std::min(s[i], s[i + (1 << k)]);
  }
 }
 // 塊の [x, y) の最小。x < y。
 u32 blocks(int x, int y) const {
  const int k= std::bit_width(unsigned(y - x)) - 1;
  const u32* row= st.get() + size_t(k) * nb;
  return std::min(row[x], row[y - (1 << k)]);
 }
 // 塊 b の lane [lo, hi] の最小。
 u32 inblock(int b, int lo, int hi) const {
  const __m256i* p= (const __m256i*)(v.get() + (size_t(b) << LB));
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
 rmq_block_scan::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
