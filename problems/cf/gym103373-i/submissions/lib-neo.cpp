// 重み付き (グラフ的 + 横断)。NeoLibrary の weighted_matroid_intersection と、GraphicMatroid、TransversalMatroid
// (直せる辺を左、作業員を右に置いた二部グラフ) を使い、直す辺の数 k ごとの最大を出す。
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include "neo/combinatorial_optimization/matroid_intersection.hpp"
#include "neo/combinatorial_optimization/matroids.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n, m;
 cin >> n >> m;
 vector<long long> a(n), cost(m);
 for(int i= 0; i < n; ++i) cin >> a[i];
 vector<array<int, 2>> es(m);
 for(int i= 0; i < m; ++i) {
  int u, v;
  cin >> u >> v, --u, --v;
  es[i]= {u, v};
  cost[i]= sqrt(a[u] + a[v]);
 }
 int w;
 cin >> w;
 vector<array<int, 2>> be;
 for(int i= 0; i < w; ++i) {
  int x;
  cin >> x;
  for(int j= 0; j < x; ++j) {
   int b;
   cin >> b, --b;
   be.push_back({b, i});
  }
 }
 auto [best, I]= weighted_matroid_intersection(GraphicMatroid(n, es), TransversalMatroid(m, w, be), cost);
 for(int i= 1; i < n; ++i) cout << (i > 1 ? " " : "") << (i < (int)best.size() ? best[i] : -1);
 cout << '\n';
 return 0;
}
