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
 auto tmp= sps::colorings_using_exactly_k_colors_num<long long>(n, es);
 long long ans= 0, fact= 1;
 for(int i= 0; i < min(3, n); ++i) fact*= 3 - i, ans+= fact * tmp[i + 1];
 cout << ans << '\n';
 return 0;
}
