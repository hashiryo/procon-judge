#pragma once
// block_mask_hp_bl と同じ表 (SIMD で分岐なしに作るマスクと、pre、suf、塊の sparse table を huge page に置く) で、
// クエリだけを block_scan_hp と同じ分岐の形にしたもの。両端が同じ塊なら a[塊の先頭 + ctz(mask[r - 1] & (~0 << l))]、
// 違えば suf[l]、pre[r - 1]、間の塊の min。block_scan_hp とは同じ塊の中の答え方だけが違う (AVX2 で 32 個を走査する
// 代わりに、マスクを引いてから a を引く 2 段の読み込み)。
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
namespace rmq_block_mask_hp {
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
 u32 *v, *pre, *suf, *mask, *st;
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), buf(alloc_huge((size_t(nb) << LB) * 4 + size_t(lg + 1) * nb)) {
  const size_t m= size_t(nb) << LB;
  v= buf.get(), pre= v + m, suf= pre + m, mask= suf + m, st= mask + m;
  std::copy(a.begin(), a.end(), v);
  std::fill(v + n, v + m, ~0u);
  const __m256i sign= _mm256_set1_epi32(int(0x80000000u));
  for(int b= 0; b < nb; ++b) {
   const size_t o= size_t(b) << LB;
   const u32* x= v + o;
   u32 *p= pre + o, *s= suf + o, *mk= mask + o;
   const __m256i* xv= (const __m256i*)x;
   const __m256i x0= _mm256_xor_si256(_mm256_load_si256(xv), sign), x1= _mm256_xor_si256(_mm256_load_si256(xv + 1), sign);
   const __m256i x2= _mm256_xor_si256(_mm256_load_si256(xv + 2), sign), x3= _mm256_xor_si256(_mm256_load_si256(xv + 3), sign);
   u32 c= ~0u, mm= 0;
   for(int i= 0; i < B; ++i) {
    p[i]= c= std::min(c, x[i]);
    const __m256i xi= _mm256_set1_epi32(int(x[i] ^ 0x80000000u));
    const u32 g0= u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(x0, xi)))), g1= u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(x1, xi))));
    const u32 g2= u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(x2, xi)))), g3= u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(x3, xi))));
    mk[i]= mm= (mm & ~(g0 | g1 << 8 | g2 << 16 | g3 << 24)) | 1u << i;
   }
   st[b]= c;
   c= ~0u;
   for(int i= B; i--;) s[i]= c= std::min(c, x[i]);
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
 u32 query(int l, int r) const {
  const int j= r - 1, bl= l >> LB, bj= j >> LB;
  if(bl == bj) return v[(size_t(bj) << LB) + std::countr_zero(mask[j] & (~0u << (l & (B - 1))))];
  u32 x= std::min(suf[l], pre[j]);
  if(bj - bl > 1) x= std::min(x, blocks(bl + 1, bj));
  return x;
 }
};
}
struct Solver {
 rmq_block_mask_hp::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
