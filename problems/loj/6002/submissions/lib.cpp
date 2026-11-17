// 今の Library の bipartite_matching で解く。頂点を出る側と入る側に分けた二部グラフの最大マッチングで、u の出る側と
// w の入る側が組になっていれば、道の中で u の次が w になる。前の頂点の無い頂点から次をたどって道を出す。
#include <cstdio>
#include <string>
#include <vector>
#include "mylib/graph/BipartiteGraph.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 BipartiteGraph bg(n, n);
 for(int i= 0, u, v; i < m; ++i) {
  if(scanf("%d %d", &u, &v) != 2) return 1;
  bg.add_edge(u - 1, n + v - 1);
 }
 auto [mc, mate]= bipartite_matching(bg);
 std::vector<int> nx(n, -1);
 std::vector<char> head(n, 1);
 for(int u= 0; u < n; ++u)
  if(mate[u] >= 0) nx[u]= mate[u] - n, head[mate[u] - n]= 0;
 std::string out;
 int cnt= 0;
 for(int v= 0; v < n; ++v)
  if(head[v]) {
   ++cnt;
   for(int x= v; x >= 0; x= nx[x]) out+= std::to_string(x + 1), out+= nx[x] >= 0 ? ' ' : '\n';
  }
 out+= std::to_string(cnt), out+= '\n';
 fputs(out.c_str(), stdout);
}
