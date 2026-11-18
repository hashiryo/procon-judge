// lib.cpp を NeoLibrary の HeavyLightDecomposition と Rerooting (どちらも木は辺の列で渡す) で書いたもの。dp(0, u) (0 を根にした
// ときの u の部分木の値) を使い、根から下への値は HLD の位置の順 (行きがけ順) に回って作る。
#include <array>
#include <iostream>
#include <vector>
#include "neo/graph/HeavyLightDecomposition.hpp"
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n, q;
 cin >> n >> q;
 vector<int> a(n);
 for(int i= 0; i < n; ++i) cin >> a[i];
 vector<array<int, 2>> es(n - 1);
 vector<long long> z1(n - 1), z2(n - 1);
 vector<vector<int>> adje(n);
 for(int i= 0; i < n - 1; ++i) {
  cin >> es[i][0] >> es[i][1] >> z1[i] >> z2[i], --es[i][0], --es[i][1];
  adje[es[i][0]].push_back(i), adje[es[i][1]].push_back(i);
 }
 HeavyLightDecomposition hld(n, es, 0);
 auto put_edge= [&](int, int e, long long x) { return max(0ll, x - z1[e] - z2[e]); };
 auto op= [](long long l, long long r) { return l + r; };
 auto put_vertex= [&](int v, long long x) { return x + a[v]; };
 Rerooting<long long> dp(n, es, put_edge, op, 0ll, put_vertex);
 vector<long long> d1(n), d2(n), f(n);
 for(int i= 0; i < n; ++i) {
  int v= hld.to_vertex(i), p= hld.parent(v);
  for(int e: adje[v]) {
   int u= es[e][0] ^ es[e][1] ^ v;
   if(u == p) continue;
   d1[u]= d1[v], d2[u]= d2[v];
   if(es[e][0] == u) d1[u]+= z1[e], d2[u]+= z2[e];
   else d2[u]+= z1[e], d1[u]+= z2[e];
   f[u]= f[v] + min(dp(0, u), z1[e] + z2[e]);
  }
 }
 while(q--) {
  int u, v;
  cin >> u >> v, --u, --v;
  int l= hld.lca(u, v);
  cout << dp[l] + f[u] + f[v] - f[l] * 2 - (d1[u] - d1[l] + d2[v] - d2[l]) << '\n';
 }
 return 0;
}
