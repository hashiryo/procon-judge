// 今の Library の bipartite_matching と DulmageMendelsohn で解く。推移閉包を bitset で作り、頂点を出る側と入る側に分けた
// 二部グラフの最小点被覆 C を取ると、出る側も入る側も C に入らない頂点が最大反鎖になる (König の定理と Dilworth の定理)。
// 3 行目は、頂点 v と比べられる頂点と v 自身を除いた残りで、最大反鎖の大きさ (頂点の数 - 最大マッチング) が K - 1 なら 1。
#include <bitset>
#include <cstdio>
#include <string>
#include <vector>
#include "mylib/graph/BipartiteGraph.hpp"
#include "mylib/graph/DulmageMendelsohn.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::vector<int>> g(n);
 std::vector<int> indeg(n, 0), ord;
 for(int i= 0, u, v; i < m; ++i) {
  if(scanf("%d %d", &u, &v) != 2) return 1;
  g[u - 1].push_back(v - 1), ++indeg[v - 1];
 }
 for(int v= 0; v < n; ++v)
  if(!indeg[v]) ord.push_back(v);
 for(int i= 0; i < (int)ord.size(); ++i)
  for(int w: g[ord[i]])
   if(!--indeg[w]) ord.push_back(w);
 std::vector<std::bitset<128>> reach(n);
 for(int i= n; i--;)
  for(int w: g[ord[i]]) reach[ord[i]][w]= 1, reach[ord[i]]|= reach[w];
 BipartiteGraph bg(n, n);
 for(int u= 0; u < n; ++u)
  for(int w= 0; w < n; ++w)
   if(reach[u][w]) bg.add_edge(u, n + w);
 DulmageMendelsohn dm(bg);
 const std::vector<int> vc= dm.min_vertex_cover();
 const int k= n - (int)vc.size();
 std::vector<char> in(2 * n, 0);
 for(int x: vc) in[x]= 1;
 std::string pick(n, '0'), can(n, '0');
 for(int v= 0; v < n; ++v)
  if(!in[v] && !in[n + v]) pick[v]= '1';
 for(int v= 0; v < n; ++v) {
  std::vector<int> id(n, -1), rest;
  for(int u= 0; u < n; ++u)
   if(u != v && !reach[u][v] && !reach[v][u]) id[u]= rest.size(), rest.push_back(u);
  const int r= rest.size();
  BipartiteGraph h(r, r);
  for(int a: rest)
   for(int b: rest)
    if(reach[a][b]) h.add_edge(id[a], r + id[b]);
  auto [mc, mate]= bipartite_matching(h);
  if(r - (int)mc.size() == k - 1) can[v]= '1';
 }
 printf("%d\n%s\n%s\n", k, pick.c_str(), can.c_str());
}
