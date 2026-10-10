#pragma once
// 行きがけ順の列の上の RMQ で O(1) に答える LCA。位置 i (1 以上) の頂点の親の位置を A[i] に置くと、u と v の位置を
// x < y として、lca は A[x + 1, y] の最小の位置の頂点になる (u が v の祖先なら u の位置が、そうでなければ lca から
// v へ向かう子の親として lca の位置が範囲に現れ、範囲の頂点はどれも lca の真の子孫なので、親の位置は lca より前に
// 行かない)。行きがけ順は、入力の保証 par[i] < i を使って、番号の大きい順に部分木の大きさを足し、小さい順に子の
// 位置を部分木の大きさずつ詰めて配る (隣接リストは組まない)。RMQ は yosupo-staticrmq の block16_fused と同じで、
// 16 個の塊の左右からの累積と塊の sparse table、塊の中は AVX2 で 1 行を走査する。A はその表の a の写しの場所に直に
// 書く。表は huge page に置く。
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
namespace lca_rmq {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
template <class T> using Buf= std::unique_ptr<T[], FreeDeleter>;
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
template <class T> Buf<T> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(T) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return Buf<T>(static_cast<T*>(p));
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
// 列 v (長さ n) の RMQ。v を書いてから build を呼ぶ。
struct Rmq {
 static constexpr int LB= 4, B= 1 << LB;
 int n, nb, lg;
 Buf<u32> buf;
 u32 *v, *pre, *suf, *st;
 explicit Rmq(int n): n(n), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), buf(alloc_huge<u32>((size_t(nb) << LB) * 3 + size_t(lg + 1) * nb)) {
  const size_t m= size_t(nb) << LB;
  v= buf.get(), pre= v + m, suf= pre + m, st= suf + m;
  std::fill(v + n, v + m, ~0u);
 }
 void build() {
  const __m256i ones= _mm256_set1_epi32(-1), seven= _mm256_set1_epi32(7), zero= _mm256_setzero_si256();
  for(int b= 0; b < nb; ++b) {
   const size_t o= size_t(b) << LB;
   const __m256i x0= _mm256_load_si256((const __m256i*)(v + o)), x1= _mm256_load_si256((const __m256i*)(v + o + 8));
   const __m256i p0= prefix8(x0, ones), p1= _mm256_min_epu32(prefix8(x1, ones), _mm256_permutevar8x32_epi32(p0, seven));
   const __m256i s1= suffix8(x1, ones), s0= _mm256_min_epu32(suffix8(x0, ones), _mm256_permutevar8x32_epi32(s1, zero));
   _mm256_store_si256((__m256i*)(pre + o), p0), _mm256_store_si256((__m256i*)(pre + o + 8), p1);
   _mm256_store_si256((__m256i*)(suf + o), s0), _mm256_store_si256((__m256i*)(suf + o + 8), s1);
   st[b]= u32(_mm256_cvtsi256_si32(s0));
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
 // [l, r) の最小。l < r。
 u32 query(int l, int r) const {
  const int j= r - 1, bl= l >> LB, bj= j >> LB;
  if(bl == bj) return inblock(bl, l & (B - 1), j & (B - 1));
  u32 x= std::min(suf[l], pre[j]);
  if(bj - bl > 1) x= std::min(x, blocks(bl + 1, bj));
  return x;
 }
};
struct Tree {
 Buf<int> buf;
 int *pos, *vert;
 Rmq rmq;
 Tree(int n, const std::vector<int>& par): buf(alloc_huge<int>(3 * size_t(n))), rmq(n) {
  int* sz= buf.get();
  pos= sz + n, vert= pos + n;
  std::fill(sz, sz + n, 1);
  for(int i= n - 1; i > 0; --i) sz[par[i]]+= sz[i];
  // 小さい順に位置を配る。sz[i] は i を配ったあと使わないので、i の最初の子を置く位置 (nxt) に書き換えて使い回す。
  u32* a= rmq.v;
  pos[0]= 0, vert[0]= 0, sz[0]= 1, a[0]= ~0u;
  for(int i= 1; i < n; ++i) {
   const int p= par[i], x= sz[p];
   sz[p]+= sz[i], sz[i]= x + 1;
   pos[i]= x, vert[x]= i, a[x]= u32(pos[p]);
  }
  rmq.build();
 }
 int lca(int u, int v) const {
  if(u == v) return u;
  const int x= pos[u], y= pos[v];
  return vert[rmq.query(std::min(x, y) + 1, std::max(x, y) + 1)];
 }
};
}
struct Solver {
 lca_rmq::Tree t;
 Solver(int n, const vector<int>& par): t(n, par) {}
 int lca(int u, int v) const { return t.lca(u, v); }
};
