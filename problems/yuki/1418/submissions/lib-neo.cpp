// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。
#include <array>
#include <iostream>
#include <vector>
#include "neo/base/engine/tree/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 using Data= array<long long, 2>;
 auto put_edge= [&](int, int, const Data& d) { return d; };
 auto op= [&](const Data& l, const Data& r) { return Data{l[0] + r[0], l[1] + r[1]}; };
 auto put_vertex= [&](int, Data d) { return ++d[0], d[1]+= d[0], d; };
 long long ans= 0;
 for(auto [_, x]: Rerooting<Data>(N, es, put_edge, op, Data{0, 0}, put_vertex)) ans+= x;
 cout << ans << '\n';
 return 0;
}
