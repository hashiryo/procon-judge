// NeoLibrary の PrimalDual (neo/combinatorial_optimization/PrimalDual.hpp) で、頂点 1 から n へ流せるだけ流す。
#include <iostream>
#include "neo/combinatorial_optimization/PrimalDual.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n, m;
 cin >> n >> m;
 PrimalDual<long long, long long> g(n);
 while(m--) {
  int s, t;
  long long c, w;
  cin >> s >> t >> c >> w;
  g.add_edge(s - 1, t - 1, c, w);
 }
 const auto [f, c]= g.flow(0, n - 1);
 cout << f << ' ' << c << '\n';
}
