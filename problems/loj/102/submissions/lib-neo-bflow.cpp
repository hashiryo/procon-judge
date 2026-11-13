// NeoLibrary の b-flow の型 MinCostFlow (neo/flow/MinCostFlow.hpp、cost scaling) の flow(s, t) で、頂点 1 から n へ
// 流せるだけ流す (最小費用最大流)。
#include <iostream>
#include "neo/flow/MinCostFlow.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n, m;
 cin >> n >> m;
 MinCostFlow<long long, long long> g(n);
 while(m--) {
  int s, t;
  long long c, w;
  cin >> s >> t >> c >> w;
  g.add_edge(s - 1, t - 1, c, w);
 }
 const long long f= g.flow(0, n - 1);
 cout << f << ' ' << (long long)g.cost() << '\n';
}
