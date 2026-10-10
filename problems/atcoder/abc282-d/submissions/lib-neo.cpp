// NeoLibrary の bipartite_coloring (neo/graph/bipartite_coloring.hpp) で解く。二部グラフでなければ 0。二部なら、頂点 u と
// 結んでも二部のままの相手は、u と同じ連結成分で同じ色の頂点 (u を含む) 以外なので、成分と色ごとの頂点の数 c について
// Σ c (N - c) / 2 から、もとからある辺の数 M を引く。連結成分は BFS で番号を振る。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/graph/bipartite_coloring.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::array<int, 2>> es(m);
 std::vector<std::vector<int>> adj(n);
 for(auto& [u, v]: es) {
  if(scanf("%d %d", &u, &v) != 2) return 1;
  --u, --v, adj[u].push_back(v), adj[v].push_back(u);
 }
 const auto col= bipartite_coloring(n, es);
 if(col.empty()) return puts("0"), 0;
 std::vector<int> comp(n, -1), q;
 std::vector<std::array<long long, 2>> cnt;
 for(int s= 0; s < n; ++s)
  if(comp[s] < 0) {
   cnt.push_back({0, 0});
   comp[s]= (int)cnt.size() - 1, q.assign(1, s);
   for(size_t i= 0; i < q.size(); ++i)
    for(int w: adj[q[i]])
     if(comp[w] < 0) comp[w]= comp[s], q.push_back(w);
   for(int v: q) ++cnt.back()[col[v]];
  }
 long long ans= 0;
 for(auto& c: cnt) ans+= c[0] * (n - c[0]) + c[1] * (n - c[1]);
 printf("%lld\n", ans / 2 - m);
}
