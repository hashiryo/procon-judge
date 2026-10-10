#pragma once
// 比べるための O(log N) の基準。下から積む非再帰のセグメント木で、葉を t[N + i] に置き、t[i] = min(t[2i], t[2i + 1])。
// クエリは両端を葉から 1 段ずつ上げ、はみ出した側の節点を拾う。表は 2N 個で 4 MB。
#include <algorithm>
#include <memory>
#include <vector>
namespace rmq_segtree {
using u32= unsigned;
struct Table {
 int n;
 std::unique_ptr<u32[]> t;
 explicit Table(const std::vector<u32>& a): n(a.size()), t(new u32[2 * size_t(n)]) {
  std::copy(a.begin(), a.end(), t.get() + n);
  for(int i= n; --i > 0;) t[i]= std::min(t[2 * i], t[2 * i + 1]);
 }
 u32 query(int l, int r) const {
  u32 x= ~0u;
  for(l+= n, r+= n; l < r; l>>= 1, r>>= 1) {
   if(l & 1) x= std::min(x, t[l++]);
   if(r & 1) x= std::min(x, t[--r]);
  }
  return x;
 }
};
}
struct Solver {
 rmq_segtree::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
