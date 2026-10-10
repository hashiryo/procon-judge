// NeoLibrary の bipartite_coloring (neo/graph/bipartite_coloring.hpp) で解く。X_{A_i} ≠ X_{B_i} を満たす 0 と 1 の列があるのは、
// 辺 (A_i, B_i) のグラフが二部グラフのとき (A_i = B_i の自己ループがあれば二部でない)。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/graph/bipartite_coloring.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::array<int, 2>> es(m);
 for(auto& e: es)
  if(scanf("%d", &e[0]) != 1) return 1;
 for(auto& e: es)
  if(scanf("%d", &e[1]) != 1) return 1;
 for(auto& e: es) --e[0], --e[1];
 puts(bipartite_coloring(n, es).empty() ? "No" : "Yes");
}
