// lib.cpp を NeoLibrary の HeavyLightDecomposition、Rerooting (どちらも木は辺の列で渡す)、WaveletMatrix で書いたもの。
// 今の Library の wm.count(l, r, u) (u 未満の個数) は、NeoLibrary の count(l, r, 0, u)。
#include <array>
#include <iostream>
#include <vector>
#include "neo/data_structure/WaveletMatrix.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 HeavyLightDecomposition hld(N, es, 0);
 vector<int> a(N);
 for(int v= N; v--;) a[hld.to_seq(v)]= v;
 WaveletMatrix<int> wm(a);
 auto put_edge= [&](int v, int e, long long d) -> long long {
  int u= es[e][0] ^ es[e][1] ^ v;
  if(u == hld.parent(v)) {
   auto [l, r]= hld.subtree(v);
   return d + u - wm.count(l, r, 0, u);
  } else {
   auto [l, r]= hld.subtree(u);
   return d + wm.count(l, r, 0, u);
  }
 };
 auto op= [&](long long l, long long r) { return l + r; };
 auto put_vertex= [&](int, long long d) { return d; };
 Rerooting<long long> dp(N, es, put_edge, op, 0ll, put_vertex);
 for(int i= 0; i < N; ++i) cout << dp[i] + i << " \n"[i == N - 1];
 return 0;
}
