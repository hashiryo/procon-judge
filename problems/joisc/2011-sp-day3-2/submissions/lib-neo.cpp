// lib.cpp を NeoLibrary の FunctionalGraph と SegmentTree<monoid::AddSum<int>> で書いたもの。今の Library の path_upto_cycle (閉路に
// 入るまでと閉路 1 周) は、path(x, dist_to_cycle(x) + cycle_length(x) - 1) で取る。
#include <iostream>
#include <vector>
#include "neo/base/ds/range/segtree/AddSum.hpp"
#include "neo/base/ds/tree/FunctionalGraph.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<int> A(N);
 for(int i= 0; i < N; i++) cin >> A[i], --A[i];
 FunctionalGraph g(A);
 SegmentTree<monoid::AddSum<int>> seg(N);
 for(int i= 0; i < N; ++i) {
  cout << seg.get(g.to_seq(i)) << '\n';
  g.path(i, g.dist_to_cycle(i) + g.cycle_length(i) - 1, [&](int l, int r, long long) { seg.apply(l, r, 1); });
 }
 return 0;
}
