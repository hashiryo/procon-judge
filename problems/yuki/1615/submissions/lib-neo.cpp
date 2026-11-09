// 重み付き二部マッチング (非想定解)。NeoLibrary の MinCostFlow (neo/flow/MinCostFlow.hpp) で、売り値を負の費用にした最小費用
// 循環流として解く。s から買い手、商品から t へ容量 1 の辺を張り、t から s へ戻す辺を足して b を 0 にすると、流れが
// マッチングに当たり、費用の和の符号を反転したものが総売り上げの最大になる。
#include <iostream>
#include "neo/flow/MinCostFlow.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, M, K, L;
 cin >> N >> M >> K >> L;
 const int s= N + M, t= s + 1;
 MinCostFlow<int, long long> g(N + M + 2);
 for(int i= 0; i < L; i++) {
  int X, Y, Z;
  cin >> X >> Y >> Z, X--, Y--;
  g.add_edge(X, N + Y, 1, -(1LL << Z));
 }
 for(int i= 0; i < N; i++) g.add_edge(s, i, 1, 0);
 for(int j= 0; j < M; j++) g.add_edge(N + j, t, 1, 0);
 g.add_edge(t, s, min(N, M), 0);
 g.solve();
 cout << (long long)-g.cost() << '\n';
 return 0;
}
