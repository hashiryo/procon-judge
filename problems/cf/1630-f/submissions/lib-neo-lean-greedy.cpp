// 診断用。lib-neo-lean の最初のマッチングを、Karp-Sipser から貪欲 (左の頂点を番号の順に見て空いている最初の隣と組ませる) に
// 替えたもの (bm_lean.hpp の min_path_cover<false, true>)。lib-neo-lean との差が、密なグラフで Karp-Sipser が辺を 2 回ずつ
// なめる分になる。比べ終えたら消す。
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
  const int w= bm_lean::min_path_cover<false, true>(2 * n, es).size();
  out+= std::to_string(n - w), out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
