// 今の Library の bipartite_edge_coloring で解く。行を左、数を右に置き、マスごとに辺を張る (多重辺になる)。M 正則なので
// M 色で塗れて、色をそのまま列の番号にする。
#include <cstdio>
#include <vector>
#include "mylib/graph/BipartiteGraph.hpp"
#include "mylib/graph/bipartite_edge_coloring.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 BipartiteGraph bg(n, n, n * m);
 std::vector<int> val(n * m);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j) {
   if(scanf("%d", &val[i * m + j]) != 1) return 1;
   bg[i * m + j]= {i, n + val[i * m + j] - 1};
  }
 const auto color= bipartite_edge_coloring(bg);
 std::vector<std::vector<int>> b(n, std::vector<int>(m));
 for(int e= 0; e < n * m; ++e) b[e / m][color[e]]= val[e];
 puts("Yes");
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j) printf("%d%c", b[i][j], j + 1 == m ? '\n' : ' ');
}
