// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。
#include <array>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/engine/tree/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 vector<array<int, 2>> es(n - 1);
 vector<double> w(n - 1), p(n);
 for(int i= 0; i < n - 1; ++i) cin >> es[i][0] >> es[i][1] >> w[i], --es[i][0], --es[i][1], w[i]/= 100;
 for(int i= 0; i < n; ++i) cin >> p[i], p[i]/= 100;
 auto put_edge= [&](int, int e, double d) { return d * w[e]; };
 auto op= [](double l, double r) { return 1 - (1 - l) * (1 - r); };
 auto put_vertex= [&](int v, double d) { return 1 - (1 - d) * (1 - p[v]); };
 double ans= 0;
 for(double x: Rerooting<double>(n, es, put_edge, op, 0., put_vertex)) ans+= x;
 cout << fixed << setprecision(6) << ans << '\n';
 return 0;
}
