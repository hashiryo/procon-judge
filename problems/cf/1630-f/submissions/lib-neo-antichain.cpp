// NeoLibrary の max_antichain (neo/algo/optimize/dp/path_cover.hpp) で解く。lib-neo.cpp と同じ辺を、推移閉包を
// 前提にしない max_antichain に渡す。推移的な関係を全部の組で渡すときに、min_path_cover (BipartiteMatching) と
// max_antichain (頂点を通り抜ける辺を足した網の MaxFlow) がどれだけ違うかを CI で見るために置いた。
#include <array>
#include <cstdio>
#include <string>
#include <vector>
#include "common.hpp"
#include "neo/algo/optimize/dp/path_cover.hpp"
int main() {
 int t;
 if(scanf("%d", &t) != 1) return 1;
 std::vector<int> pos(50001, -1);
 std::string out;
 while(t--) {
  int n;
  if(scanf("%d", &n) != 1) return 1;
  std::vector<int> a(n);
  for(int& x: a)
   if(scanf("%d", &x) != 1) return 1;
  std::vector<std::array<int, 2>> es;
  cf1630f_edges(a, pos, [&](int u, int v) { es.push_back({u, v}); });
  const int w= max_antichain(2 * n, es).size();
  out+= std::to_string(n - w), out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
