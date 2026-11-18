// 二部マッチング (分割 + 分割)。NeoLibrary の matroid_intersection と PartitionMatroid を使う。
#include <iostream>
#include <vector>
#include <numeric>
#include "neo/combinatorial_optimization/matroid_intersection.hpp"
#include "neo/combinatorial_optimization/matroids.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 for(int m, n; cin >> m >> n && m;) {
  vector<int> b(m), r(n), pl, pr;
  for(int i= 0; i < m; ++i) cin >> b[i];
  for(int i= 0; i < n; ++i) cin >> r[i];
  for(int i= 0; i < m; ++i)
   for(int j= 0; j < n; ++j)
    if(gcd(b[i], r[j]) > 1) pl.push_back(i), pr.push_back(j);
  cout << matroid_intersection(PartitionMatroid(pl, vector(m, 1)), PartitionMatroid(pr, vector(n, 1))).size() << '\n';
 }
 return 0;
}
