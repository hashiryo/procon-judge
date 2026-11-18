// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 vector<array<int, 2>> es(n - 1);
 vector<int> w(n - 1);
 for(int i= 0; i < n - 1; ++i) cin >> es[i][0] >> es[i][1] >> w[i];
 auto put_edge= [&](int, int e, int d) { return d + w[e]; };
 auto op= [&](int l, int r) { return max(l, r); };
 auto put_vertex= [&](int, int d) { return d; };
 Rerooting<int> dp(n, es, put_edge, op, 0, put_vertex);
 cout << *max_element(dp.begin(), dp.end()) << '\n';
 return 0;
}
