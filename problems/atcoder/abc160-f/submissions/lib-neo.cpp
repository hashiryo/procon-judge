// lib.cpp の Graph と Rerooting を NeoLibrary の Rerooting (木は辺の列で渡す) に替えたもの。二項係数は今の Library の
// FactorialPrecalculation と ModInt のまま。
#include <array>
#include <iostream>
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/counting/FactorialPrecalculation.hpp"
#include "neo/base/engine/tree/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<int(1e9 + 7)>;
 using F= FactorialPrecalculation<Mint>;
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 using Data= pair<int, Mint>;
 auto put_edge= [&](int, int, const Data& d) { return d; };
 auto op= [&](const Data& l, const Data& r) { return Data{l.first + r.first, F::nCr(l.first + r.first, l.first) * l.second * r.second}; };
 auto put_vertex= [&](int, const Data& d) { return Data{d.first + 1, d.second}; };
 for(auto [_, x]: Rerooting<Data>(N, es, put_edge, op, Data{0, 1}, put_vertex)) cout << x << '\n';
 return 0;
}
