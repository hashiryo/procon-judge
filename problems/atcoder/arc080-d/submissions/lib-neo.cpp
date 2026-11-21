// https://atcoder.jp/contests/arc080/tasks/arc080_d
// NeoLibrary の min_weight_perfect_matching (neo/algo/optimize/matching/weighted_matching.hpp) で解く。
#include <iostream>
#include <tuple>
#include <vector>
#include "neo/algo/optimize/matching/weighted_matching.hpp"
#include "neo/base/core/nt/is_prime.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<int> x(N);
 for(int i= 0; i < N; ++i) cin >> x[i];
 vector<int> vs= {x[0]};
 for(int i= 1; i < N; ++i)
  if(x[i] - x[i - 1] > 1) vs.push_back(x[i - 1] + 1), vs.push_back(x[i]);
 vs.push_back(x[N - 1] + 1);
 const int m= vs.size();
 vector<tuple<int, int, int>> es;
 for(int i= 0; i < m; ++i)
  for(int j= i + 1; j < m; ++j) {
   const int k= vs[j] - vs[i];
   es.emplace_back(i, j, k & 1 ? is_prime(k) ? 1 : 3 : 2);
  }
 cout << min_weight_perfect_matching(m, es)->weight << '\n';
 return 0;
}
