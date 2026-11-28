#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include "neo/algo/compute/geo/min_enclosing_circle.hpp"
#include "neo/base/core/geo/Circle.hpp"
using namespace std;
// 二部グラフの最大マッチング (左 0..L-1、右は adj の値 0..R-1)。左の番号の小さい順に、
// 相手の列が辞書順で最小になるものを選ぶ。mylib の bipartite_matching<true> と同じ手順。
namespace lex_matching_internal {
vector<int> solve(int L, int R, vector<vector<int>> adj) {
 vector<int> mate(L + R, -1);
 for(int l= 0; l < L; ++l) {
  sort(adj[l].begin(), adj[l].end());
  for(int& r: adj[l]) r+= L;
 }
 {
  vector<int> a, p, q(L);
  for(bool u= true; u;) {
   u= false, a.assign(L, -1), p.assign(L, -1);
   int t= 0;
   for(int l= L; l--;)
    if(mate[l] == -1) q[t++]= a[l]= p[l]= l;
   for(int i= 0; i < t; ++i)
    if(int l= q[i], x; mate[a[l]] == -1)
     for(int r: adj[l]) {
      if(x= mate[r]; x == -1) {
       for(u= true; r != -1; l= p[l]) mate[r]= l, swap(mate[l], r);
       break;
      }
      if(p[x] == -1) a[q[t++]= x]= a[p[x]= l];
     }
  }
 }
 vector<char> a(L, 1);
 for(int l= 0; l < L; ++l)
  if(int r= mate[l], v= l; r != -1) {
   vector<int> p(L, mate[v]= mate[r]= -1), c(L, 0);
   for(p[v]= -2;;) {
    if(c[v] == (int)adj[v].size()) v= p[v];
    else if(int u= mate[r= adj[v][c[v]++]]; u == -1) {
     for(; r != -1; v= p[v]) mate[r]= v, swap(mate[v], r);
     break;
    } else if(a[u] && p[u] == -1) p[u]= v, v= u;
   }
   a[l]= 0;
  }
 return mate;
}
}
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 auto mec_r= [](const vector<Point<R>>& qs) {
  auto s= min_enclosing_circle(qs);
  if(s.m <= 1) return R(0);
  if(s.m == 2) return R(dist(s.a, s.b) / 2);
  return circumscribed_circle(s.a, s.b, s.c).r;
 };
 int n, m;
 cin >> n >> m;
 vector<R> rs(n);
 for(int i= 0; i < n; i++) cin >> rs[i];
 vector<vector<int>> adj(m);
 for(int j= 0; j < m; j++) {
  int p;
  cin >> p;
  vector<Point<R>> ps(p);
  for(int k= 0; k < p; k++) cin >> ps[k];
  R r= mec_r(ps);
  for(int i= 0; i < n; i++)
   if(sgn(rs[i] - r) >= 0) adj[j].push_back(i);
 }
 auto mate= lex_matching_internal::solve(m, n, adj);
 int cnt= 0;
 for(int l= 0; l < m; ++l) cnt+= mate[l] != -1;
 if(cnt < m) cout << "NG" << '\n';
 else
  for(int l= 0; l < m; ++l) cout << mate[l] - m + 1 << '\n';
 return 0;
}
