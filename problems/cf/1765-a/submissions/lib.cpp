// 今の Library の bipartite_matching で解く。文書の包含の順序 (common.hpp) は推移的なので、文書を出る側と入る側に分けた
// 二部グラフの最大マッチングで、組になった j の次を j' にすれば、頂点を共有しない道がそのまま最小の数の鎖になる。
#include <vector>
#include "common.hpp"
#include "mylib/graph/BipartiteGraph.hpp"
int main() {
 CF1765A p;
 if(!p.read()) return 1;
 const int m= p.m;
 BipartiteGraph bg(m, m);
 p.for_each_pair([&](int j, int k) { bg.add_edge(j, m + k); });
 auto [mc, mate]= bipartite_matching(bg);
 std::vector<int> nx(m, -1);
 std::vector<char> head(m, 1);
 for(int j= 0; j < m; ++j)
  if(mate[j] >= 0) nx[j]= mate[j] - m, head[mate[j] - m]= 0;
 std::vector<std::vector<int>> chains;
 for(int j= 0; j < m; ++j)
  if(head[j]) {
   auto& c= chains.emplace_back();
   for(int x= j; x >= 0; x= nx[x]) c.push_back(x);
  }
 p.print(chains);
}
