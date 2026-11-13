// NeoLibrary の PrimalDual (neo/combinatorial_optimization/PrimalDual.hpp) で、頂点 0 から V - 1 へ F まで流す。F 流せなければ -1。
#include <iostream>
#include "neo/combinatorial_optimization/PrimalDual.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int V, E, F;
 cin >> V >> E >> F;
 PrimalDual<int, long long> g(V);
 while(E--) {
  int u, v, c, d;
  cin >> u >> v >> c >> d;
  g.add_edge(u, v, c, d);
 }
 const auto [f, c]= g.flow(0, V - 1, F);
 cout << (f == F ? c : -1) << '\n';
}
