// NeoLibrary の min_path_cover (neo/algo/optimize/cover/path_cover.hpp) で解く。依頼 i のあとに依頼 j へ間に合う
// (t_i + d(p_i, p_j) <= t_j) 組を辺にした DAG の、頂点を共有しない道の最小の本数が答え。最短距離の三角不等式で関係は
// 推移的なので、鎖の被覆 (サンタが依頼のない家を通ってよい) と同じ本数になる。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/algo/optimize/cover/path_cover.hpp"
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
  std::vector<std::array<int, 2>> es;
  for(int i= 0; i < l; ++i)
   for(int j= 0; j < l; ++j)
    if(i != j && d[p[i]][p[j]] < INF && t[i] + d[p[i]][p[j]] <= t[j]) es.push_back({i, j});
  printf("%d\n", (int)min_path_cover(l, es).size());
 }
}
