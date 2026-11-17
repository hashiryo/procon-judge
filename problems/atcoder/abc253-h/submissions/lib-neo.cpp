#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/graph.hpp"
using namespace std;
using Z= ZMod<998244353>;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n, m;
 cin >> n >> m;
 sps::Edges es;
 for(int i= 0, u, v; i < m; ++i) cin >> u >> v, es.push_back({u - 1, v - 1});
 auto tree= sps::tree<Z>(n, es);
 auto f= sps::egf_T(tree);
 Z fact= 1, iv= Z(1) / m, pw= 1;
 for(int i= 1; i < n; i++) cout << f[n - i] * (fact*= i) * (pw*= iv) << '\n';
 return 0;
}
