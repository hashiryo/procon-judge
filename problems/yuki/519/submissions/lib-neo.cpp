// NeoLibrary の max_weight_matching (neo/algo/optimize/matching/weighted_matching.hpp) で解く。行列の成分をそのまま辺に
// して渡す (多重辺は重い方を使い、自己ループは使わない)。
#include <iostream>
#include <tuple>
#include <vector>
#include "neo/algo/optimize/matching/weighted_matching.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 vector<tuple<int, int, long long>> es;
 es.reserve(n * n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) {
   long long F;
   cin >> F;
   es.emplace_back(i, j, F);
  }
 cout << max_weight_matching(n, es).weight << '\n';
 return 0;
}
