#pragma once
// disjoint sparse table を 1 本の配列に段ごとに並べたもの。段 p (1 <= p <= bit_width(N - 1)) は幅 2^p の塊を半分に
// 分け、左半分には真ん中までの最小 (右から左へ累積)、右半分には真ん中からの最小 (左から右へ累積) を持つ。段 1 は a
// そのもの。クエリは j = r - 1 として p = bit_width((l ^ j) | 1) の段の l と j を引く。l と j は同じ塊の左と右に
// 分かれるので、2 つの min が答えになる。l == j のときは p = 1 で a[l] を 2 回引く形になり、分岐は無い。表は 19 段、
// 38 MB で、0 で埋める手間を省くため new で取る。累積は依存の鎖になるので 1 個ずつ回す。
#include <algorithm>
#include <bit>
#include <memory>
#include <vector>
namespace rmq_disjoint {
using u32= unsigned;
struct Table {
 int n, levels;
 std::unique_ptr<u32[]> t;
 explicit Table(const std::vector<u32>& a): n(a.size()), levels(std::max(1, int(std::bit_width(unsigned(n - 1))))), t(new u32[size_t(levels) * n]) {
  std::copy(a.begin(), a.end(), t.get());
  for(int p= 2; p <= levels; ++p) {
   u32* row= t.get() + size_t(p - 1) * n;
   const int half= 1 << (p - 1);
   for(int b= 0; b < n; b+= 2 * half) {
    const int mid= std::min(b + half, n), e= std::min(b + 2 * half, n);
    u32 cur= ~0u;
    for(int i= mid; i-- > b;) row[i]= cur= std::min(cur, a[i]);
    cur= ~0u;
    for(int i= mid; i < e; ++i) row[i]= cur= std::min(cur, a[i]);
   }
  }
 }
 u32 query(int l, int r) const {
  const int j= r - 1, p= std::bit_width(unsigned(l ^ j) | 1u);
  const u32* row= t.get() + size_t(p - 1) * n;
  return std::min(row[l], row[j]);
 }
};
}
struct Solver {
 rmq_disjoint::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
