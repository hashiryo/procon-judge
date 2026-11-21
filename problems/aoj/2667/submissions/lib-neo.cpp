// パス総和、部分木加算。lib.cpp を NeoLibrary の HeavyLightDecomposition と SegmentTree<monoid::AddSum> (区間加算と区間和) で
// 書いたもの。辺の値は子の側の頂点に置き、道の和は lca を除く (path の edge を真にする)。
#include <iostream>
#include <array>
#include <vector>
#include "neo/base/ds/range/segtree/AddSum.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, Q;
 cin >> N >> Q;
 vector<array<int, 2>> es(N - 1);
 for(auto& e: es) cin >> e[0] >> e[1];
 HeavyLightDecomposition tree(N, es, 0);
 SegmentTree<monoid::AddSum<long long>> seg(N);
 while(Q--) {
  int op, a, b;
  cin >> op >> a >> b;
  if(op) {
   auto [l, r]= tree.subtree(a);
   if(l + 1 < r) seg.apply(l + 1, r, b);
  } else {
   long long ans= 0;
   tree.path(a, b, [&](int x, int y) { ans+= x <= y ? seg.prod(x, y + 1) : seg.prod(y, x + 1); }, true);
   cout << ans << '\n';
  }
 }
 return 0;
}
