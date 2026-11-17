#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/graph.hpp"
#include "neo/set_power_series/relaxed.hpp"
using namespace std;
using Z= ZMod<998244353>;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n, m, p;
 cin >> n >> m >> p;
 int N= 1 << n;
 sps::Edges es;
 for(int i= 0, u, v; i < m; ++i) cin >> u >> v, es.push_back({u - 1, v - 1});
 vector<Z> w(N);
 for(int i= 0; i < n; ++i) cin >> w[1 << i];
 sps::subset_zeta(w);
 for(int s= N; --s;) w[s]= w[s].pow(p);
 auto comp= sps::connected_component_num(n, es);
 auto odd= sps::odd_deg_num(n, es);
 vector<Z> f(N);
 for(int s= N; --s;)
  if(comp[s] != 1 || odd[s] != 0) f[s]= w[s];
 auto phi= [&](int s, Z& x) { x/= w[s]; };
 auto ans= sps::semi_relaxed_convolve<Z>(f, Z(1), phi);
 cout << ans.back() << '\n';
 return 0;
}
