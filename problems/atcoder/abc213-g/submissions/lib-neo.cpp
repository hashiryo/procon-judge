#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/count/graph/graph.hpp"
using namespace std;
using Z= ZMod<998244353>;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n, m;
 cin >> n >> m;
 sps::Edges es;
 for(int i= 0, u, v; i < m; ++i) cin >> u >> v, es.push_back({u - 1, v - 1});
 auto x= sps::graph<Z>(n, es);
 auto y= sps::connected_graph<Z>(n, es);
 int all= (1 << n) - 1;
 for(int k= 1; k < n; ++k) {
  Z ans= 0;
  for(int s= 1; s <= all; s+= 2)
   if((s >> k) & 1) ans+= y[s] * x[all ^ s];
  cout << ans << '\n';
 }
 return 0;
}
