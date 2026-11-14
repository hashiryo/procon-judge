// NeoLibrary の KaryOptimization (neo/combinatorial_optimization/KaryOptimization.hpp) で解く。x_i を候補の番号 (M 値) にし、
// 1 変数の項を C、2 変数の項を W_{i,j} |A_{i,a} - A_{j,b}| にする。A は候補ごとに昇順なので、2 変数の項は Monge。
#include <cstdlib>
#include <iostream>
#include <vector>
#include "neo/combinatorial_optimization/KaryOptimization.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n, m;
 cin >> n >> m;
 vector<vector<long long>> a(n, vector<long long>(m)), c(n, vector<long long>(m));
 for(int i= 0; i < n; ++i)
  for(int k= 0; k < m; ++k) cin >> a[i][k] >> c[i][k];
 KaryOptimization<long long> ko(n, m);
 for(int i= 0; i < n; ++i) ko.add(i, c[i]);
 for(int i= 0; i < n; ++i)
  for(int j= i + 1; j < n; ++j) {
   long long w;
   cin >> w;
   ko.add(i, j, [&](int x, int y) { return llabs(a[i][x] - a[j][y]) * w; });
  }
 cout << *ko.solve() << '\n';
}
