// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。辺の向こうの頂点は両端の xor で引く。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "neo/base/engine/tree/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, K;
 cin >> N >> K;
 vector<array<int, 2>> es(N - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 vector<int> cnt(N);
 for(int i= 0, D; i < K; ++i) cin >> D, ++cnt[--D];
 using Data= array<int, 2>;
 auto put_edge= [&](int v, int e, Data d) {
  if(cnt[es[e][0] ^ es[e][1] ^ v] || d[0]) d[0]+= 2, d[1]+= 1;
  return d;
 };
 auto op= [&](const Data& l, const Data& r) { return Data{l[0] + r[0], max(l[1], r[1])}; };
 auto put_vertex= [&](int, const Data& d) { return d; };
 for(auto [a, b]: Rerooting<Data>(N, es, put_edge, op, Data{0, 0}, put_vertex)) cout << a - b << '\n';
 return 0;
}
