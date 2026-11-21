// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) と ZMod で書いたもの。
#include <array>
#include <iostream>
#include <tuple>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/engine/tree/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<1000000007>;
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 vector<Mint> w(N - 1);
 for(int i= 0; i < N - 1; ++i) cin >> es[i][0] >> es[i][1] >> w[i], --es[i][0], --es[i][1];
 using Data= tuple<int, Mint, Mint>;
 auto put_edge= [&](int, int e, const Data& d) {
  auto [d0, d1, d2]= d;
  return Data{d0 + 1, d1 + w[e] * Mint(d0 + 1), d2 + w[e] * (d1 + d1 + w[e] * Mint(d0 + 1))};
 };
 auto op= [&](const Data& l, const Data& r) {
  auto [l0, l1, l2]= l;
  auto [r0, r1, r2]= r;
  return Data{l0 + r0, l1 + r1, l2 + r2};
 };
 auto put_vertex= [&](int, const Data& d) { return d; };
 Mint ans= 0;
 for(auto [_, __, x]: Rerooting<Data>(N, es, put_edge, op, Data{0, Mint(), Mint()}, put_vertex)) ans+= x;
 cout << ans / Mint(2) << '\n';
 return 0;
}
