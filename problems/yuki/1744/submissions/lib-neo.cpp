// NeoLibrary の BipartiteMatching (neo/graph/BipartiteMatching.hpp) で書いたもの。辺 i を除いても最大マッチングの大きさが変わらないのは、
// i がどの最大マッチングにも入るわけではないとき (edge_kind(i) != 2)。
#include <array>
#include <cstdio>
#include <string>
#include <vector>
#include "neo/graph/BipartiteMatching.hpp"
int main() {
 int n, m, l;
 if(scanf("%d %d %d", &n, &m, &l) != 3) return 1;
 std::vector<std::array<int, 2>> es(l);
 for(auto& [a, b]: es) {
  if(scanf("%d %d", &a, &b) != 2) return 1;
  --a, --b;
 }
 BipartiteMatching bm(n, m, es);
 std::string out;
 for(int i= 0; i < l; ++i) out+= bm.edge_kind(i) == 2 ? "No\n" : "Yes\n";
 std::fputs(out.c_str(), stdout);
}
