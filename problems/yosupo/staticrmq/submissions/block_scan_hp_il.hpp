#pragma once
// block_scan_hp の表の作り方だけを変えたもの。塊の中の累積の最小 (pre と suf) は 1 要素ごとに前の値の min を取る
// 依存の鎖で、1 本ずつ回すと min の待ちが続く。そこで塊を 4 つずつまとめ、4 本の鎖を同じループで進める。クエリは
// block_scan_hp と同じ。
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
namespace rmq_block_scan_hp_il {
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
 u32 *v, *pre, *suf, *st;
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), buf(alloc_huge((size_t(nb) << LB) * 3 + size_t(lg + 1) * nb)) {
  const size_t m= size_t(nb) << LB;
  v= buf.get(), pre= v + m, suf= pre + m, st= suf + m;
  std::copy(a.begin(), a.end(), v);
  std::fill(v + n, v + m, ~0u);
  int b= 0;
  for(; b + 4 <= nb; b+= 4) {
   const size_t o= size_t(b) << LB;
   const u32* x= v + o;
   u32 *p= pre + o, *s= suf + o;
   u32 c0= ~0u, c1= ~0u, c2= ~0u, c3= ~0u;
   for(int i= 0; i < B; ++i) {
    p[i]= c0= std::min(c0, x[i]), p[i + B]= c1= std::min(c1, x[i + B]);
    p[i + 2 * B]= c2= std::min(c2, x[i + 2 * B]), p[i + 3 * B]= c3= std::min(c3, x[i + 3 * B]);
   }
   st[b]= c0, st[b + 1]= c1, st[b + 2]= c2, st[b + 3]= c3;
   c0= c1= c2= c3= ~0u;
   for(int i= B; i--;) {
    s[i]= c0= std::min(c0, x[i]), s[i + B]= c1= std::min(c1, x[i + B]);
    s[i + 2 * B]= c2= std::min(c2, x[i + 2 * B]), s[i + 3 * B]= c3= std::min(c3, x[i + 3 * B]);
   }
  }
  for(; b < nb; ++b) {
   const size_t o= size_t(b) << LB;
   const u32* x= v + o;
   u32 *p= pre + o, *s= suf + o;
   u32 c= ~0u;
   for(int i= 0; i < B; ++i) p[i]= c= std::min(c, x[i]);
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
 rmq_block_scan_hp_il::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
