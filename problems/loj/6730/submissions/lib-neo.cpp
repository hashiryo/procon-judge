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
 cout << sps::two_edge_connected_graph<Z>(n, es).back() << '\n';
 return 0;
}
