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
 int n;
 cin >> n;
 sps::Edges es;
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) {
   bool c;
   cin >> c;
   if(c && i < j) es.push_back({i, j});
  }
 auto f= sps::chromatic_polynomial<Z>(n, es);
 for(int i= 0; i <= n; ++i) cout << f[i] << " \n"[i == n];
 return 0;
}
