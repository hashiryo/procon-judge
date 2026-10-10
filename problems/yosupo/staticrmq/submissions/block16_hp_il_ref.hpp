#pragma once
// block16_hp_il から a の写しを除き、渡された配列をそのまま読むもの。Solver は配列より先に壊れる前提で、ハーネスの
// a はそれを満たす。写す 2 MB と、それを huge page の領域に用意する手間が無くなる代わりに、配列の境は glibc の
// 確保のまま 16 byte で、塊の 64 byte はキャッシュの 2 行にまたがる。半端な最後の塊だけは、最大値で埋めた写しを
// 持って読む。ライブラリにするなら、配列を借りる形を口にするかどうかの判断の材料にする。
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
namespace rmq_block16_hp_il_ref {
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
 const u32* v;
 Buf buf;
 u32 *pre, *suf, *st;
 alignas(64) u32 tail[B];
 const u32* last;  // 最後の塊を読むときの先頭。半端なら tail
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), v(a.data()), buf(alloc_huge((size_t(nb) << LB) * 2 + size_t(lg + 1) * nb)) {
  const size_t m= size_t(nb) << LB, o_last= size_t(nb - 1) << LB;
  pre= buf.get(), suf= pre + m, st= suf + m;
  std::fill(tail, tail + B, ~0u);
  std::copy(v + o_last, v + n, tail);
  last= (n & (B - 1)) ? tail : v + o_last;
  const int full= n >> LB;  // a から直に読める塊の数
  int b= 0;
  for(; b + 4 <= full; b+= 4) {
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
   const u32* x= b == nb - 1 ? last : v + o;
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
 // last が自分の tail を指すことがあるので、写しも移動もさせない。
 Table(const Table&)= delete;
 Table& operator=(const Table&)= delete;
 // 塊の [x, y) の最小。x < y。
 u32 blocks(int x, int y) const {
  const int k= std::bit_width(unsigned(y - x)) - 1;
  const u32* row= st + size_t(k) * nb;
  return std::min(row[x], row[y - (1 << k)]);
 }
 // 塊 b の lane [lo, hi] の最小。
 u32 inblock(int b, int lo, int hi) const {
  const __m256i* p= (const __m256i*)(b == nb - 1 ? last : v + (size_t(b) << LB));
  const __m256i vlo= _mm256_set1_epi32(lo), vhi= _mm256_set1_epi32(hi), eight= _mm256_set1_epi32(8);
  __m256i idx= _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7), m= _mm256_set1_epi32(-1);
  for(int t= 0; t < B / 8; ++t) {
   const __m256i out= _mm256_or_si256(_mm256_cmpgt_epi32(vlo, idx), _mm256_cmpgt_epi32(idx, vhi));
   m= _mm256_min_epu32(m, _mm256_or_si256(_mm256_loadu_si256(p + t), out));
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
 rmq_block16_hp_il_ref::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
