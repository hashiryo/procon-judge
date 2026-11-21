// lib.cpp の Graph と Rerooting を NeoLibrary の Rerooting (木は辺の列で渡す) に替えたもの。dp2(i, u) (i を根にしたときの u の
// 部分木の値) を使う。根付き木のハッシュは今の Library の ModInt (法 2^61 - 1)、Pointwise、rng のまま。
#include <array>
#include <iostream>
#include <set>
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/misc/Pointwise.hpp"
#include "mylib/misc/rng.hpp"
#include "neo/base/engine/tree/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<array<int, 2>> A(N - 1), B(N);
 vector<vector<int>> adj(N + 1);
 for(auto& [a, b]: A) cin >> a >> b, --a, --b;
 for(auto& [a, b]: B) cin >> a >> b, --a, --b, adj[a].push_back(b), adj[b].push_back(a);
 using Mint= ModInt<(1ll << 61) - 1>;
 using K= Pointwise<Mint, Mint>;
 vector<K> hash(N + 1);
 for(auto& x: hash) x= {rng(2, Mint::mod() - 1), rng(2, Mint::mod() - 1)};
 using T= pair<int, K>;
 auto put_edge= [&](int, int, const T& d) { return T(d.first, d.second + hash[d.first]); };
 auto op= [&](const T& l, const T& r) { return T(max(l.first, r.first), l.second * r.second); };
 auto put_vertex= [&](int, const T& d) { return T(d.first + 1, d.second); };
 Rerooting<T> dp1(N, A, put_edge, op, T(0, 1), put_vertex), dp2(N + 1, B, put_edge, op, T(0, 1), put_vertex);
 set<K> S;
 for(auto&& x: dp1) S.insert(x.second);
 int ans= -1;
 for(int i= 0; i <= N; ++i)
  if(adj[i].size() == 1)
   if(S.count(dp2(i, adj[i][0]).second)) {
    ans= i + 1;
    break;
   }
 cout << ans << '\n';
 return 0;
}
