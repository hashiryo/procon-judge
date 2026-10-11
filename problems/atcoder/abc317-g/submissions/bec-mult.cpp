// 二部グラフの辺彩色で解く (核は procon-judge の _shared/flow/bec_mult.hpp。多重辺を辺と重複度にまとめ、Euler 閉路と
// 完全マッチングで分ける)。行を左、数を右に置き、マスごとに辺を張る。M 正則なので M 色で塗れて、色をそのまま列の番号にする。
#include <cstdio>
#include <vector>
#include "_shared/flow/bec_mult.hpp"
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
 const auto color= bec_mult::edge_coloring<64>(n, n, e);
 std::vector<std::vector<int>> b(n, std::vector<int>(m));
 for(int k= 0; k < n * m; ++k) b[k / m][color[k]]= val[k];
 puts("Yes");
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j) printf("%d%c", b[i][j], j + 1 == m ? '\n' : ' ');
}
