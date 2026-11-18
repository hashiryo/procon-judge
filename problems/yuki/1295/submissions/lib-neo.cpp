// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。辺の向こうの頂点は両端の xor で引く。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 vector<vector<int>> adj(N);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b, adj[a].push_back(b), adj[b].push_back(a);
 vector<int> mx(N), mx2(N), mn(N);
 for(int i= 0; i < N; ++i) {
  auto vs= adj[i];
  sort(vs.begin(), vs.end());
  int n= vs.size();
  mx[i]= vs[n - 1], mn[i]= vs[0];
  mx2[i]= n > 1 ? vs[n - 2] : -1;
 }
 auto put_edge= [&](int v, int e, int d) {
  int u= es[e][0] ^ es[e][1] ^ v;
  if(d >= 4 || (d == 3 && v != mx[u])) return 4;
  if(d == 0 && v == mn[u]) return 0;
  if(u == mx[v] || u == mn[v]) return 2;
  if(u == mx2[v]) return 3;
  return 4;
 };
 auto op= [&](int l, int r) { return l + r; };
 auto put_vertex= [&](int, int d) { return d; };
 for(auto x: Rerooting<int>(N, es, put_edge, op, 0, put_vertex)) cout << (x <= 2 ? "Yes" : "No") << '\n';
 return 0;
}
