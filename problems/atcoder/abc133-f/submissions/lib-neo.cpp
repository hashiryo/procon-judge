// lib-patricia.cpp の木を NeoLibrary の HeavyLightDecomposition にしたもの。根からの道の色ごとの本数と長さの和は、今の Library の
// 永続なパトリシア木で持つ (NeoLibrary に永続な木はまだ無い)。根から順に版を作るのは、HLD の位置の順 (親が子より前) に回す。
#include <iostream>
#include <array>
#include <vector>
#include "mylib/data_structure/SegmentTree_Patricia.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, Q;
 cin >> N >> Q;
 vector<array<int, 2>> es(N - 1);
 vector<int> c(N - 1), d(N - 1);
 for(int i= 0; i < N - 1; ++i) cin >> es[i][0] >> es[i][1] >> c[i] >> d[i], --es[i][0], --es[i][1];
 HeavyLightDecomposition hld(N, es, 0);
 vector<int> pe(N, -1);  // 頂点 -> 親への辺
 for(int i= 0; i < N - 1; ++i) pe[hld.parent(es[i][0]) == es[i][1] ? es[i][0] : es[i][1]]= i;
 using PerArr= SegmentTree_Patricia<int, true, 17>;
 vector<PerArr> Arr1(N), Arr2(N);
 vector<int> dep(N);
 for(int i= 1; i < N; ++i) {
  const int u= hld.to_vertex(i), v= hld.parent(u), e= pe[u];
  Arr1[u]= Arr1[v], Arr2[u]= Arr2[v];
  Arr1[u][c[e]]+= 1, Arr2[u][c[e]]+= d[e];
  dep[u]= dep[v] + d[e];
 }
 for(int i= 0; i < Q; i++) {
  int x, y, u, v;
  cin >> x >> y >> u >> v;
  int lca= hld.lca(--u, --v);
  cout << dep[u] + dep[v] - 2 * dep[lca] + y * (Arr1[u].get(x) + Arr1[v].get(x) - 2 * Arr1[lca].get(x)) - (Arr2[u].get(x) + Arr2[v].get(x) - 2 * Arr2[lca].get(x)) << '\n';
 }
 return 0;
}
