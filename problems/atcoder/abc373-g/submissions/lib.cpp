// 今の Library の NetworkSimplex で解く。P から Q へ容量 1、費用が距離 (double) の辺を N^2 本張り、P に供給、Q に需要を 1 ずつ
// 置いた b-flow を解いて、流れた辺を組にする。
#include <cmath>
#include <cstdio>
#include <vector>
#include "mylib/optimization/NetworkSimplex.hpp"
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<long long> x(2 * n), y(2 * n);
 for(int i= 0; i < 2 * n; ++i)
  if(scanf("%lld %lld", &x[i], &y[i]) != 2) return 1;
 using MCF= NetworkSimplex<int, double>;
 MCF g;
 auto P= g.add_vertices(n), Q= g.add_vertices(n);
 for(int i= 0; i < n; ++i) g.add_supply(P[i], 1), g.add_demand(Q[i], 1);
 std::vector<std::vector<MCF::EdgePtr>> es(n, std::vector<MCF::EdgePtr>(n));
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) {
   const long long dx= x[i] - x[n + j], dy= y[i] - y[n + j];
   es[i][j]= g.add_edge(P[i], Q[j], 0, 1, std::sqrt(double(dx * dx + dy * dy)));
  }
 g.b_flow();
 std::vector<int> r(n, -1);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j)
   if(es[i][j].flow()) r[i]= j;
 for(int i= 0; i < n; ++i) printf("%d%c", r[i] + 1, i + 1 == n ? '\n' : ' ');
}
