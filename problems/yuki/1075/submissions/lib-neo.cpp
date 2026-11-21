// lib.cpp を NeoLibrary の HeavyLightDecomposition、Rerooting (どちらも木は辺の列で渡す)、ZMod で書いたもの。dp(i, j) (i を根に
// したときの j の部分木の値) を使い、j が 0 を根にしたときの i の親かどうかを HLD で見る。値は長さ K の vector。
#include <array>
#include <iostream>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"
#include "neo/base/engine/tree/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<1000000007>;
 int N, K;
 cin >> N >> K;
 vector<array<int, 2>> es(N - 1);
 vector<vector<int>> adj(N);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b, adj[a].push_back(b), adj[b].push_back(a);
 HeavyLightDecomposition tree(N, es, 0);
 using Data= vector<Mint>;
 auto put_edge= [&](int, int, const Data& d) { return d; };
 auto op= [&](Data l, const Data& r) {
  for(int i= K; i--;) l[i]*= r[i];
  return l;
 };
 auto put_vertex= [&](int, Data d) {
  for(int i= 1; i < K; ++i) d[i]+= d[i - 1];
  return d;
 };
 Rerooting<Data> dp(N, es, put_edge, op, Data(K, Mint(1)), put_vertex);
 Mint ans= 0;
 for(int i= 0; i < N; ++i) {
  Data a(K, Mint(1));
  for(int j: adj[i]) {
   const Data& b= dp(i, j);
   if(j == tree.parent(i)) {
    for(int k= 1; k < K; ++k) a[k]*= b[k - 1];
    a[0]= 0;
   } else
    for(int k= 0; k < K; ++k) a[k]*= b[k];
  }
  for(int k= 1; k < K; ++k) a[k]+= a[k - 1];
  ans+= a[K - 1];
 }
 cout << ans << '\n';
 return 0;
}
