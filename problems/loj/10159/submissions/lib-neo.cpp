// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。dp(v, u) (v を根にしたときの u の部分木の値) を使う。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 vector<array<int, 2>> es(n - 1);
 vector<vector<int>> adj(n);
 for(auto& [a, b]: es) cin >> a >> b, adj[a].push_back(b), adj[b].push_back(a);
 auto put_edge= [&](int, int, int d) { return d + 1; };
 auto op= [&](int l, int r) { return max(l, r); };
 auto put_vertex= [&](int, int d) { return d; };
 Rerooting<int> dp(n, es, put_edge, op, 0, put_vertex);
 int di= 0;
 for(int v= n; v--;) di= max(di, dp[v]);
 vector<int> ans;
 for(int v= 0; v < n; ++v) {
  if(adj[v].size() == 1) {
   if(dp[v] == di) ans.push_back(v);
   continue;
  }
  int mx= 0, mx2= 0;
  for(int u: adj[v]) {
   int d= dp(v, u) + 1;
   if(mx < d) mx2= mx, mx= d;
   else if(mx2 < d) mx2= d;
  }
  if(mx + mx2 == di) ans.push_back(v);
 }
 for(auto a: ans) cout << a << '\n';
 return 0;
}
