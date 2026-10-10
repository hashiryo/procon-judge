// 今の Library の MaxFlow (Dinic) で解く。頂点 v を入口 v と出口 v + N に分け、入口から出口へ容量 c_v (1 と N は無限)、
// 元の辺ごとに両向きに出口から入口へ容量無限の辺を張る。壁は、最小カットで入口が s の側、出口が t の側にある頂点。
#include <cstdio>
#include <vector>
#include "mylib/optimization/MaxFlow.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::array<int, 2>> es(m);
 for(auto& [a, b]: es)
  if(scanf("%d %d", &a, &b) != 2) return 1;
 std::vector<long long> c(n + 1);
 for(int i= 1; i <= n; ++i)
  if(scanf("%lld", &c[i]) != 1) return 1;
 const long long INF= 1LL << 60;
 MaxFlow<Dinic<long long>> g(2 * n + 1);
 for(int v= 1; v <= n; ++v) g.add_edge(v, v + n, v == 1 || v == n ? INF : c[v]);
 for(auto [a, b]: es) g.add_edge(a + n, b, INF), g.add_edge(b + n, a, INF);
 const long long C= g.maxflow(1 + n, n);
 const auto cut= g.mincut(1 + n);
 std::vector<int> wall;
 for(int v= 2; v < n; ++v)
  if(cut[v] && !cut[v + n]) wall.push_back(v);
 printf("%lld\n%d\n", C, (int)wall.size());
 for(size_t i= 0; i < wall.size(); ++i) printf("%d%c", wall[i], i + 1 == wall.size() ? '\n' : ' ');
 if(wall.empty()) printf("\n");
}
