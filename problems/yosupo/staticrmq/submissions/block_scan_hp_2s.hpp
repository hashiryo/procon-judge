#pragma once
// block_scan_hp_bl から suf も除き、塊の中の累積を持たないもの。両端の塊を毎回 AVX2 で走査する。l の塊は lane
// [l, 両端が同じ塊なら r - 1、違えば塊の末尾]、r - 1 の塊は lane [両端が同じ塊なら l、違えば塊の先頭, r - 1] を見る。
// 同じ塊なら 2 つとも同じ答えになる。間の塊は block_scan_hp_bl と同じく、範囲を寄せて毎回引いて値を選ぶ。クエリの
// 中に分岐は無い。表は a の写しと塊の sparse table だけで 3 MB ほど。累積の表をばらばらに引く代わりに、両端の塊の
// 128 byte を続けて読む。
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
namespace rmq_block_scan_hp_2s {
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
 static constexpr int LB= 5, B= 1 << LB;
 int n, nb, lg;
 Buf buf;
 u32 *v, *st;
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), buf(alloc_huge((size_t(nb) << LB) + size_t(lg + 1) * nb)) {
  const size_t m= size_t(nb) << LB;
  v= buf.get(), st= v + m;
  std::copy(a.begin(), a.end(), v);
  std::fill(v + n, v + m, ~0u);
  for(int b= 0; b < nb; ++b) st[b]= *std::min_element(v + (size_t(b) << LB), v + (size_t(b + 1) << LB));
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
  // 条件式で値を選ぶと GCC が分岐に戻すので、全部引いてからマスクで潰す。
  const u32 same= 0u - u32(bl == bj), none= 0u - u32(bj - bl <= 1);
  const int x= std::min(bl + 1, nb - 1), y= std::max(bj, x + 1);
  const u32 sl= inblock(bl, l & (B - 1), (j & (B - 1)) | int(u32(B - 1) & ~same)), sj= inblock(bj, int(u32(l & (B - 1)) & same), j & (B - 1));
  const u32 mid= blocks(x, y) | none;
  return std::min(std::min(sl, sj), mid);
 }
};
}
struct Solver {
 rmq_block_scan_hp_2s::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
