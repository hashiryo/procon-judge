// NeoLibrary の min_path_cover (neo/algo/optimize/dp/path_cover.hpp) で解く。頂点を 2 つに分けた順序 (common.hpp)
// の最大反鎖の大きさ w を求め、答えは n - w。関係は推移的なので、頂点を共有しない道の本数が w と等しく、推移的な関係を全部の
// 組で渡すときは min_path_cover が速い。
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
  const int w= min_path_cover(2 * n, es).size();
  out+= std::to_string(n - w), out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
