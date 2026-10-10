#pragma once
// block_scan と同じ形で、同じ塊の中のクエリだけを 32 bit のマスクで答える。塊の先頭から i まで読んだときの単調な
// スタック (後ろのどれよりも大きくない位置の集合) を、塊の中の位置の bit で mask[i] に持つ。[l, j] の最小は、
// mask[j] のうち l 以上で一番下の bit の位置にあるので、a[塊の先頭 + ctz(mask[j] & (~0 << (l - 塊の先頭)))] で
// 引ける。クエリの両端が別の塊なら block_scan と同じく suf、pre、塊の sparse table の min。表は block_scan に
// mask の N 個を足して 9 MB ほど。スタックを作るときの取り出しは値で分岐する。
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <memory>
#include <vector>
namespace rmq_block_mask {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
using Buf= std::unique_ptr<u32[], FreeDeleter>;
inline Buf alloc(size_t n) { return Buf((u32*)std::aligned_alloc(64, (n * sizeof(u32) + 63) & ~size_t(63))); }
struct Table {
 static constexpr int LB= 5, B= 1 << LB;
 int n, nb, lg;
 Buf v, pre, suf, mask, st;
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), v(alloc(size_t(nb) << LB)), pre(alloc(size_t(nb) << LB)), suf(alloc(size_t(nb) << LB)), mask(alloc(size_t(nb) << LB)), st(alloc(size_t(lg + 1) * nb)) {
  std::copy(a.begin(), a.end(), v.get());
  std::fill(v.get() + n, v.get() + (size_t(nb) << LB), ~0u);
  for(int b= 0; b < nb; ++b) {
   const size_t o= size_t(b) << LB;
   const u32* x= v.get() + o;
   u32 *p= pre.get() + o, *s= suf.get() + o, *mk= mask.get() + o;
   u32 c= ~0u, m= 0;
   for(int i= 0; i < B; ++i) {
    p[i]= c= std::min(c, x[i]);
    while(m && x[31 - std::countl_zero(m)] > x[i]) m&= ~(1u << (31 - std::countl_zero(m)));
    mk[i]= m|= 1u << i;
   }
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
 u32 query(int l, int r) const {
  const int j= r - 1, bl= l >> LB, bj= j >> LB;
  if(bl == bj) return v[(j & ~(B - 1)) + std::countr_zero(mask[j] & (~0u << (l & (B - 1))))];
  u32 x= std::min(suf[l], pre[j]);
  if(bj - bl > 1) x= std::min(x, blocks(bl + 1, bj));
  return x;
 }
};
}
struct Solver {
 rmq_block_mask::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
