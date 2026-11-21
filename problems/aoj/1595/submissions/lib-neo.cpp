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
 auto put_edge= [&](int, int, int d) { return d + 1; };
 auto op= [&](int l, int r) { return max(l, r); };
 auto put_vertex= [&](int, int d) { return d; };
 for(int x: Rerooting<int>(N, es, put_edge, op, 0, put_vertex)) cout << 2 * (N - 1) - x << '\n';
 return 0;
}
