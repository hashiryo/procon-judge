// 今の Library の StronglyConnectedComponents と bipartite_matching で解く。品名を頂点、品名 i の 2 文字目と品名 j の
// 1 文字目が同じなら i から j への辺を張る。NG リストの文字列はこのグラフの歩道で、同じ品名を何度通ってもよい。強連結成分は
// 1 本でまとめて覆えるので、縮めた DAG の到達可能性の順序を最小の数の鎖で覆う本数が答え。推移閉包を bitset で作り、出る側と
// 入る側に分けた二部グラフの最大マッチングを、成分の数から引く。
#include <array>
#include <bitset>
#include <cstdio>
#include <vector>
#include "mylib/graph/BipartiteGraph.hpp"
#include "mylib/graph/StronglyConnectedComponents.hpp"
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<std::array<char, 3>> s(n);
 for(auto& x: s)
  if(scanf("%2s", x.data()) != 1) return 1;
 Graph g(n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j)
   if(s[i][1] == s[j][0]) g.add_edge(i, j);
 StronglyConnectedComponents scc(g);
 const int k= scc.size();
 const Graph dag= scc.dag(g);
 std::vector<std::vector<int>> out(k);
 std::vector<int> indeg(k, 0), ord;
 for(auto [a, b]: dag) out[a].push_back(b), ++indeg[b];
 for(int v= 0; v < k; ++v)
  if(!indeg[v]) ord.push_back(v);
 for(int i= 0; i < (int)ord.size(); ++i)
  for(int w: out[ord[i]])
   if(!--indeg[w]) ord.push_back(w);
 std::vector<std::bitset<676>> reach(k);
 for(int i= k; i--;)
  for(int w: out[ord[i]]) reach[ord[i]][w]= 1, reach[ord[i]]|= reach[w];
 BipartiteGraph bg(k, k);
 for(int u= 0; u < k; ++u)
  for(int w= 0; w < k; ++w)
   if(reach[u][w]) bg.add_edge(u, k + w);
 auto [mc, mate]= bipartite_matching(bg);
 printf("%d\n", k - (int)mc.size());
}
