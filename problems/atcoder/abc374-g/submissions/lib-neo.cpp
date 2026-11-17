// NeoLibrary の strongly_connected_components と max_antichain (neo/combinatorial_optimization/path_cover.hpp) で解く。
// 品名を頂点、品名 i の 2 文字目と品名 j の 1 文字目が同じなら i から j への辺を張る。NG リストの文字列はこのグラフの歩道で、
// 強連結成分は 1 本でまとめて覆えるので、縮めた DAG の到達可能性の順序を覆う最小の鎖の数 (= 最大反鎖の大きさ) が答え。
// 推移閉包は作らず、縮めた DAG の辺をそのまま渡す。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/combinatorial_optimization/path_cover.hpp"
#include "neo/graph/strongly_connected_components.hpp"
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<std::array<char, 3>> s(n);
 for(auto& x: s)
  if(scanf("%2s", x.data()) != 1) return 1;
 std::vector<std::vector<int>> by_first(26);
 for(int j= 0; j < n; ++j) by_first[s[j][0] - 'A'].push_back(j);
 std::vector<std::array<int, 2>> es;
 for(int i= 0; i < n; ++i)
  for(int j: by_first[s[i][1] - 'A']) es.push_back({i, j});
 const auto [k, comp]= strongly_connected_components(n, es);
 std::vector<std::array<int, 2>> dag;
 for(auto [u, v]: es)
  if(comp[u] != comp[v]) dag.push_back({comp[u], comp[v]});
 printf("%d\n", (int)max_antichain(k, dag).size());
}
