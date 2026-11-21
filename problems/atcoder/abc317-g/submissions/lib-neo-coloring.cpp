// NeoLibrary の bipartite_edge_coloring (neo/algo/optimize/matching/bipartite_edge_coloring.hpp) で解く。行を左、数を右に
// 置き、マスごとに辺を張る。M 正則なので M 色で塗れて、色をそのまま列の番号にする。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/algo/optimize/matching/bipartite_edge_coloring.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::array<int, 2>> e(n * m);
 std::vector<int> val(n * m);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j) {
   if(scanf("%d", &val[i * m + j]) != 1) return 1;
   e[i * m + j]= {i, val[i * m + j] - 1};
  }
 const std::vector<int> color= bipartite_edge_coloring(n, n, e);
 std::vector<std::vector<int>> b(n, std::vector<int>(m));
 for(int k= 0; k < n * m; ++k) b[k / m][color[k]]= val[k];
 puts("Yes");
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j) printf("%d%c", b[i][j], j + 1 == m ? '\n' : ' ');
}
