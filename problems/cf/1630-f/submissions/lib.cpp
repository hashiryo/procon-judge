// 今の Library の bipartite_matching で解く。残すグラフが二部グラフであることは、割り切れる 3 つの値の鎖が無いことと同じ
// (比較可能グラフは完全グラフなので、三角形が無ければ 2 色で塗れる)。そういう部分集合の最大の大きさは、頂点を 2 つに分けた
// 順序 (common.hpp) の最大反鎖の大きさ w で、答えは n - w。関係は推移的なので、出る側と入る側に分けた二部グラフの最大
// マッチング M から w = 2n - M と求まる。
#include <cstdio>
#include <string>
#include <vector>
#include "common.hpp"
#include "mylib/graph/BipartiteGraph.hpp"
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
  BipartiteGraph bg(2 * n, 2 * n);
  cf1630f_edges(a, pos, [&](int u, int v) { bg.add_edge(u, 2 * n + v); });
  auto [mc, mate]= bipartite_matching(bg);
  const int w= 2 * n - (int)mc.size();
  out+= std::to_string(n - w), out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
