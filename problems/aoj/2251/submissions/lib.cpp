// 今の Library の bipartite_matching で解く。依頼 i のあとに依頼 j へ間に合う (t_i + d(p_i, p_j) <= t_j) 組を辺に
// して、依頼を出る側と入る側に分けた二部グラフの最大マッチングを取る。答えは L - (マッチングの大きさ)。
#include <cstdio>
#include <vector>
#include "mylib/graph/BipartiteGraph.hpp"
int main() {
 constexpr int INF= 1 << 29;
 for(int n, m, l; scanf("%d %d %d", &n, &m, &l) == 3 && (n | m | l);) {
  std::vector<std::vector<int>> d(n, std::vector<int>(n, INF));
  for(int i= 0; i < n; ++i) d[i][i]= 0;
  for(int i= 0, u, v, w; i < m; ++i) {
   if(scanf("%d %d %d", &u, &v, &w) != 3) return 1;
   if(w < d[u][v]) d[u][v]= d[v][u]= w;
  }
  for(int k= 0; k < n; ++k)
   for(int i= 0; i < n; ++i)
    for(int j= 0; j < n; ++j)
     if(d[i][k] + d[k][j] < d[i][j]) d[i][j]= d[i][k] + d[k][j];
  std::vector<int> p(l), t(l);
  for(int i= 0; i < l; ++i)
   if(scanf("%d %d", &p[i], &t[i]) != 2) return 1;
  BipartiteGraph bg(l, l);
  for(int i= 0; i < l; ++i)
   for(int j= 0; j < l; ++j)
    if(i != j && d[p[i]][p[j]] < INF && t[i] + d[p[i]][p[j]] <= t[j]) bg.add_edge(i, l + j);
  auto [mc, mate]= bipartite_matching(bg);
  printf("%d\n", l - (int)mc.size());
 }
}
