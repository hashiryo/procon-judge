// NeoLibrary の PrimalDual (neo/flow/PrimalDual.hpp) の slope で解く。大学と分野を頂点にし、人を大学から分野への容量 1、
// 費用 -C の辺にする (負の費用の辺は DAG なので、初めのポテンシャルは位相順に決まる)。s から t への slope の折れ点の間は
// 傾きが一定なので、i 人のときの費用は折れ点の間を線形に埋めれば求まり、その符号を反転したものが答え。
#include <iostream>
#include <vector>
#include "neo/flow/PrimalDual.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 const int A= 150, S= 0, T= 2 * A + 1;
 PrimalDual<int, long long> g(2 * A + 2);
 for(int a= 1; a <= A; ++a) g.add_edge(S, a, 1, 0), g.add_edge(A + a, T, 1, 0);
 for(int i= 0; i < n; ++i) {
  int a, b;
  long long c;
  cin >> a >> b >> c;
  g.add_edge(a, A + b, 1, -c);
 }
 const auto sl= g.slope(S, T);
 const int k= sl.back().first;
 cout << k << '\n';
 for(size_t j= 1; j < sl.size(); ++j) {
  const auto [x0, y0]= sl[j - 1];
  const auto [x1, y1]= sl[j];
  const long long d= (y1 - y0) / (x1 - x0);
  for(int x= x0 + 1; x <= x1; ++x) cout << -(y0 + d * (x - x0)) << '\n';
 }
}
