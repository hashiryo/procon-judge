// NeoLibrary の MinCostFlow (neo/flow/MinCostFlow.hpp) で、費用を -log(安全度) にした実数の費用の最小費用流として解く。費用が
// 浮動小数点数の型なので、中身はネットワーク単体法になる。総部から間諜 j へ容量 AM_j、間諜どうしは両向きに容量 M_ij、ドイツ
// 軍と連絡する間諜から終点へ容量 K の辺を張り、K 流せれば exp(-費用の和) が答え、流せなければ 0。
#include <cmath>
#include <cstdio>
#include "neo/flow/MinCostFlow.hpp"
int main() {
 int N, K;
 if(scanf("%d %d", &N, &K) != 2) return 1;
 std::vector<double> AS(N);
 std::vector<int> AM(N);
 for(auto& x: AS)
  if(scanf("%lf", &x) != 1) return 1;
 for(auto& x: AM)
  if(scanf("%d", &x) != 1) return 1;
 const int s= N, t= N + 1;
 MinCostFlow<int, double> g(N + 2);
 for(int j= 0; j < N; ++j)
  if(AM[j] > 0 && AS[j] > 0) g.add_edge(s, j, AM[j], -std::log(AS[j]));
 for(int j= 0; j < N; ++j) {
  int c;
  if(scanf("%d", &c) != 1) return 1;
  if(c) g.add_edge(j, t, K, 0.0);
 }
 for(;;) {
  int i, j;
  if(scanf("%d %d", &i, &j) != 2 || i < 0) break;
  double S;
  int M;
  if(scanf("%lf %d", &S, &M) != 2) return 1;
  if(S > 0 && M > 0) g.add_edge(i - 1, j - 1, M, -std::log(S)), g.add_edge(j - 1, i - 1, M, -std::log(S));
 }
 if(g.flow(s, t, K) < K) puts("0");
 else printf("%.10g\n", std::exp(-g.cost()));
}
