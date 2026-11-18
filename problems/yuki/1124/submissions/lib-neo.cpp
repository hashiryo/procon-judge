// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) と ZMod で書いたもの。
#include <array>
#include <iostream>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<1000000007>;
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 using Data= array<Mint, 2>;
 static constexpr Mint iv2= Mint(1) / Mint(2);
 auto put_edge= [&](int, int, const Data& d) { return Data{d[0] * iv2, d[1] * iv2}; };
 auto op= [&](const Data& l, const Data& r) { return Data{l[0] + r[0], l[1] + l[0] * r[0] * Mint(2) + r[1]}; };
 auto put_vertex= [&](int, const Data& d) { return Data{d[0] + Mint(1), d[1] + d[0] * Mint(2) + Mint(1)}; };
 Mint ans= 0;
 for(auto [_, x]: Rerooting<Data>(N, es, put_edge, op, Data{0, 0}, put_vertex)) ans+= x;
 ans*= Mint(2).pow(N - 1);
 cout << ans << '\n';
 return 0;
}
