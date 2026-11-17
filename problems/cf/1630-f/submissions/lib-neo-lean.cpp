// 診断用。bm_lean.hpp の min_path_cover<true, true> で解く。NeoLibrary の min_path_cover から、辺の列を写さない形、2 MB 以上の
// 配列を huge page に置く形、道を長さちょうどで取る形に変えたもの。最初のマッチングは Karp-Sipser のまま。比べ終えたら消す。
#include <array>
#include <cstdio>
#include <string>
#include <vector>
#include "common.hpp"
#include "bm_lean.hpp"
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
  const int w= bm_lean::min_path_cover<true, true>(2 * n, es).size();
  out+= std::to_string(n - w), out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
