// NeoLibrary の MinCostFlow (neo/flow/MinCostFlow.hpp) で、頂点 0 から V - 1 へ F 流す b-flow として解く。
#include <iostream>
#include "neo/flow/MinCostFlow.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int V, E, F;
 cin >> V >> E >> F;
 MinCostFlow<long long, long long> g(V);
 while(E--) {
  int u, v, c, d;
  cin >> u >> v >> c >> d;
  g.add_edge(u, v, c, d);
 }
 g.add_supply(0, F), g.add_supply(V - 1, -F);
 cout << (g.solve() ? (long long)g.cost() : -1) << '\n';
}
