// 重み付き (グラフ的 + 分割)。NeoLibrary の weighted_matroid_intersection は重みの和の最大を求めるので、重みの符号を
// 反転して渡し、要素数 n - 1 で打ち切る。
#include <iostream>
#include <vector>
#include <array>
#include "neo/algo/optimize/matroid/matroid_intersection.hpp"
#include "neo/algo/optimize/matroid/matroids.hpp"
using namespace std;
int main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 for(int n, m, k; cin >> n >> m >> k && n;) {
  vector<array<int, 2>> es(m);
  vector<int> part(m);
  vector<long long> w(m);
  for(int i= 0; i < m; i++) {
   int u, v, c;
   char l;
   cin >> u >> v >> c >> l, u--, v--;
   es[i]= {u, v}, part[i]= l == 'B', w[i]= -c;
  }
  auto [best, I]= weighted_matroid_intersection(GraphicMatroid(n, es), PartitionMatroid(part, {k, n - 1 - k}), w, n - 1);
  if((int)best.size() < n) cout << -1 << '\n';
  else cout << -best[n - 1] << '\n';
 }
 return 0;
}
