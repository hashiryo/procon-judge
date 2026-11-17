// NeoLibrary の max_antichain と min_path_cover (neo/combinatorial_optimization/path_cover.hpp) で解く。水の流れる向きの
// 到達可能性の順序で、祭祀点の集合は反鎖になる。1 行目と 2 行目は、入力の辺をそのまま max_antichain に渡して出す (推移閉包は
// 作らない)。3 行目は、頂点 v と比べられる頂点 (v から届くか v に届く頂点) と v 自身を除いた残りで、最大反鎖の大きさが
// K - 1 なら 1。残りの順序は元の順序を制限したものなので、推移閉包を作って残りの頂点の間の組を渡す。関係が推移的なので、
// 頂点を共有しない道の本数 (min_path_cover) が最大反鎖の大きさと等しい。
#include <array>
#include <bitset>
#include <cstdio>
#include <string>
#include <vector>
#include "neo/combinatorial_optimization/path_cover.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::array<int, 2>> es(m);
 for(auto& [u, v]: es) {
  if(scanf("%d %d", &u, &v) != 2) return 1;
  --u, --v;
 }
 const std::vector<int> anti= max_antichain(n, es);
 const int k= anti.size();
 std::string pick(n, '0'), can(n, '0');
 for(int v: anti) pick[v]= '1';
 // 推移閉包。トポロジカル順の後ろから、行き先の閉包を足す。
 std::vector<std::vector<int>> g(n);
 std::vector<int> indeg(n, 0), ord;
 for(auto [u, v]: es) g[u].push_back(v), ++indeg[v];
 for(int v= 0; v < n; ++v)
  if(!indeg[v]) ord.push_back(v);
 for(int i= 0; i < (int)ord.size(); ++i)
  for(int w: g[ord[i]])
   if(!--indeg[w]) ord.push_back(w);
 std::vector<std::bitset<128>> reach(n);
 for(int i= n; i--;)
  for(int w: g[ord[i]]) reach[ord[i]][w]= 1, reach[ord[i]]|= reach[w];
 for(int v= 0; v < n; ++v) {
  std::vector<int> id(n, -1), rest;
  for(int u= 0; u < n; ++u)
   if(u != v && !reach[u][v] && !reach[v][u]) id[u]= rest.size(), rest.push_back(u);
  std::vector<std::array<int, 2>> ce;
  for(int a: rest)
   for(int b: rest)
    if(reach[a][b]) ce.push_back({id[a], id[b]});
  if((int)min_path_cover(rest.size(), ce).size() == k - 1) can[v]= '1';
 }
 printf("%d\n%s\n%s\n", k, pick.c_str(), can.c_str());
}
