// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) と ZMod で書いたもの。
#include <array>
#include <iostream>
#include <string>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<998244353>;
 int N;
 cin >> N;
 vector<string> A(N);
 for(int i= 0; i < N; ++i) cin >> A[i];
 vector<array<int, 2>> es(N - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 using T= pair<int, Mint>;
 auto put_edge= [&](int, int, const T& d) { return d; };
 auto op= [&](const T& l, const T& r) { return T{l.first + r.first, l.second + r.second}; };
 auto put_vertex= [&](int v, T d) {
  ++d.first;
  d.second*= Mint(10).pow(A[v].length());
  d.second+= Mint(stoll(A[v])) * Mint(d.first);
  return d;
 };
 Mint ans= 0;
 for(auto [_, x]: Rerooting<T>(N, es, put_edge, op, T(0, Mint()), put_vertex)) ans+= x;
 cout << ans << "\n";
 return 0;
}
