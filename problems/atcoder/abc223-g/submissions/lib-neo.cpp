// NeoLibrary の BipartiteMatching (neo/graph/BipartiteMatching.hpp) で書いたもの。木を bipartite_coloring で塗り分けて 2 つ目の入口に渡す。
// 頂点 v を除いても最大マッチングの大きさが変わらないのは、v を空ける最大マッチングがあるとき (vertex_kind(v) != 2)。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/graph/BipartiteMatching.hpp"
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<std::array<int, 2>> es(n - 1);
 for(auto& [u, v]: es) {
  if(scanf("%d %d", &u, &v) != 2) return 1;
  --u, --v;
 }
 BipartiteMatching bm(bipartite_coloring(n, es), es);
 int ans= 0;
 for(int v= 0; v < n; ++v) ans+= bm.vertex_kind(v) != 2;
 std::printf("%d\n", ans);
}
