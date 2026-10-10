#pragma once
// block_scan_hp_bl の塊を 16 個にしたもの。同じ塊の中の走査が AVX2 の 2 本で済む代わりに、塊の sparse table が
// N / 16 * 15 段に増える (5 * 10^5 で 1.9 MB)。ほかは block_scan_hp_bl と同じで、クエリの中に分岐は無い。
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
namespace rmq_block16_hp_bl {
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
struct Table {
 static constexpr int LB= 4, B= 1 << LB;
 int n, nb, lg;
 Buf buf;
 u32 *v, *suf, *st;
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), buf(alloc_huge((size_t(nb) << LB) * 2 + size_t(lg + 1) * nb)) {
  const size_t m= size_t(nb) << LB;
  v= buf.get(), suf= v + m, st= suf + m;
  std::copy(a.begin(), a.end(), v);
  std::fill(v + n, v + m, ~0u);
  for(int b= 0; b < nb; ++b) {
   const size_t o= size_t(b) << LB;
   const u32* x= v + o;
   u32* s= suf + o;
   u32 c= ~0u;
   for(int i= B; i--;) s[i]= c= std::min(c, x[i]);
   st[b]= c;
  }
  for(int k= 0; k < lg; ++k) {
   const u32* s= st + size_t(k) * nb;
   u32* d= st + size_t(k + 1) * nb;
   for(int i= 0, len= nb - (2 << k) + 1; i < len; ++i) d[i]= std::min(s[i], s[i + (1 << k)]);
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
#pragma GCC unroll 8
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
  // 条件式で値を選ぶと、GCC が両端の同じ塊の場合を分岐に戻して suf を読み飛ばすので、全部引いてからマスクで潰す。
  const u32 same= 0u - u32(bl == bj), none= 0u - u32(bj - bl <= 1);
  const int x= std::min(bl + 1, nb - 1), y= std::max(bj, x + 1);
  const u32 sj= inblock(bj, int(u32(l & (B - 1)) & same), j & (B - 1));
  const u32 sl= suf[l] | same, mid= blocks(x, y) | none;
  return std::min(sj, std::min(sl, mid));
 }
};
}
struct Solver {
 rmq_block16_hp_bl::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
