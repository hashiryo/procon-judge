#pragma once
// 64 個ずつの塊と 64 bit のマスクだけで答える、前処理 O(N) の RMQ。mask[i] は block_mask と同じく、塊の先頭から i
// まで読んだときの単調なスタックを塊の中の位置の bit で持ったもの。塊の中の [l, j] の最小は
// a[塊の先頭 + ctz(mask[j] & (~0 << (l - 塊の先頭)))]。両端が別の塊なら、l の塊の右端の mask で l から右端まで、
// mask[j] の一番下の bit で塊の先頭から j まで、間の塊は塊ごとの最小の sparse table で引く。pre と suf を持たない
// ので、表は a の写しと mask (N 個の u64) と塊の sparse table (N / 64 * 13 段) で 6 MB ほど。その代わり、端の 2 つは
// mask を引いてから a を引く 2 段の読み込みになる。
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <memory>
#include <vector>
namespace rmq_linear64 {
using u32= unsigned;
using u64= unsigned long long;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
template <class T> using Buf= std::unique_ptr<T[], FreeDeleter>;
template <class T> Buf<T> alloc(size_t n) { return Buf<T>((T*)std::aligned_alloc(64, (n * sizeof(T) + 63) & ~size_t(63))); }
struct Table {
 static constexpr int LB= 6, B= 1 << LB;
 int n, nb, lg;
 Buf<u32> v;
 Buf<u64> mask;
 Buf<u32> st;
 explicit Table(const std::vector<u32>& a): n(a.size()), nb((n + B - 1) >> LB), lg(std::bit_width(unsigned(nb)) - 1), v(alloc<u32>(n)), mask(alloc<u64>(n)), st(alloc<u32>(size_t(lg + 1) * nb)) {
  std::copy(a.begin(), a.end(), v.get());
  for(int b= 0; b < nb; ++b) {
   const int o= b << LB, e= std::min(B, n - o);
   const u32* x= v.get() + o;
   u64* mk= mask.get() + o;
   u64 m= 0;
   for(int i= 0; i < e; ++i) {
    while(m && x[63 - std::countl_zero(m)] > x[i]) m&= ~(1ull << (63 - std::countl_zero(m)));
    mk[i]= m|= 1ull << i;
   }
   st[b]= x[std::countr_zero(m)];
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
 // 同じ塊の [l, j] の最小。
 u32 inblock(int l, int j) const { return v[(j & ~(B - 1)) + std::countr_zero(mask[j] & (~0ull << (l & (B - 1))))]; }
 u32 query(int l, int r) const {
  const int j= r - 1, bl= l >> LB, bj= j >> LB;
  if(bl == bj) return inblock(l, j);
  u32 x= std::min(inblock(l, l | (B - 1)), v[(j & ~(B - 1)) + std::countr_zero(mask[j])]);
  if(bj - bl > 1) x= std::min(x, blocks(bl + 1, bj));
  return x;
 }
};
}
struct Solver {
 rmq_linear64::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
