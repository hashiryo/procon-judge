// NeoLibrary の BipartiteMatching (neo/graph/BipartiteMatching.hpp) で書いたもの。奇数番 (0 始まりでは偶数) のオスを色 0、メスを色 1 として
// 2 つ目の入口に渡せば、頂点は元の番号のまま扱える。min_vertex_cover() は番号の小さい頂点ほど被覆に入れるので、
// 辞書順最小の最小点被覆が番号の順に返る。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/graph/BipartiteMatching.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::array<int, 2>> es(m);
 for(auto& [a, b]: es) {
  int f;
  if(scanf("%d %d %d", &a, &b, &f) != 3) return 1;
  --a, --b;
 }
 std::vector<int> color(2 * n);
 for(int v= 0; v < 2 * n; ++v) color[v]= v & 1;
 BipartiteMatching bm(color, es);
 const auto vc= bm.min_vertex_cover();
 std::printf("%d\n", (int)vc.size());
 for(int v: vc) std::printf("%d\n", v + 1);
}
