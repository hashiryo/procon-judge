// lib.cpp を NeoLibrary で書いたもの。二重連結成分は biconnected_components で求め、頂点と成分を結ぶ block-cut tree を辺の列で
// 作って HeavyLightDecomposition に渡す。自己ループは連結性に効かないので捨てる。
#include <iostream>
#include <array>
#include <vector>
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"
#include "neo/algo/decide/graph/biconnected_components.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, M;
 cin >> N >> M;
 vector<array<int, 2>> es;
 es.reserve(M);
 for(int i= 0, a, b; i < M; ++i) {
  cin >> a >> b, --a, --b;
  if(a != b) es.push_back({a, b});
 }
 const auto bc= biconnected_components(N, es);
 vector<array<int, 2>> te;
 for(int k= 0; k < bc.count; ++k)
  for(int j= bc.start[k]; j < bc.start[k + 1]; ++j) te.push_back({N + k, bc.vertex[j]});
 HeavyLightDecomposition bct(N + bc.count, te, 0);
 int Q;
 cin >> Q;
 while(Q--) {
  int x, y;
  cin >> x >> y;
  if(x == y) cout << 0 << '\n';
  else cout << bct.dist(--x, --y) / 2 - 1 << '\n';
 }
 return 0;
}
