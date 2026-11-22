// NeoLibrary の strongly_connected_components と min_chain_cover (neo/algo/optimize/cover/path_cover.hpp) で解く。
// lib-neo.cpp と同じ帰着で、縮めた DAG の到達可能性の順序を最小の数の鎖に分け、その本数を出す。min_chain_cover が流れから
// 鎖を取り出す分も CI の 4 つの環境で確かめるために、lib-neo.cpp と別に置いた。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/algo/optimize/cover/path_cover.hpp"
#include "neo/algo/decide/graph/strongly_connected_components.hpp"
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
 printf("%d\n", (int)min_chain_cover(k, dag).size());
}
