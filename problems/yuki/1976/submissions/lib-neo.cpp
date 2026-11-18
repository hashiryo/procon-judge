// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。辺ごとに dp(u, v) と dp(v, u) (片方を根にしたときのもう
// 片方の部分木の値) を使う。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 using Dat= array<int, 2>;
 using Data= array<int, 3>;
 auto put_edge= [&](int, int, const Dat& d) { return Data{d[0] + 1, 0, d[1]}; };
 auto op= [&](const Data& l, const Data& r) {
  int v[]= {l[0], l[1], r[0], r[1]};
  sort(v, v + 4);
  return Data{v[3], v[2], max(l[2], r[2])};
 };
 auto put_vertex= [&](int, const Data& d) { return Dat{d[0], max(d[2], d[0] + d[1])}; };
 Rerooting<Dat> dp(N, es, put_edge, op, Data{0, 0, 0}, put_vertex);
 int ans= 1 << 30;
 for(auto [u, v]: es) {
  int X= dp(u, v)[1], Y= dp(v, u)[1];
  ans= min(ans, max({X, Y, (X + 1) / 2 + (Y + 1) / 2 + 1}));
 }
 cout << ans << '\n';
 return 0;
}
