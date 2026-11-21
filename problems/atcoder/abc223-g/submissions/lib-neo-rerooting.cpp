// lib-rerooting.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。
#include <algorithm>
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
 using Data= array<int, 2>;
 auto put_edge= [&](int, int, const Data& d) {
  auto x= max(d[0], d[1]);
  return Data{x, d[0] + 1 - x};
 };
 auto op= [&](const Data& l, const Data& r) { return Data{l[0] + r[0], max(l[1], r[1])}; };
 auto put_vertex= [&](int, const Data& d) { return Data{d[0], d[0] + d[1]}; };
 Rerooting<Data> dp(N, es, put_edge, op, Data{0, 0}, put_vertex);
 int mx= 0, ans= 0;
 for(auto [x, y]: dp) mx= max({mx, x, y});
 for(auto [x, y]: dp) ans+= (mx == x);
 cout << ans << '\n';
 return 0;
}
