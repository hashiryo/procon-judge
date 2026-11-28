#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "mylib/geometry/Convex.hpp"
using namespace std;
// a を b の辺で 1 本ずつ切る。O(辺の本数 * 多角形の頂点数)。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using R= long double;
 int n;
 cin >> n;
 vector<Point<R>> ps(n);
 for(auto& p: ps) cin >> p;
 int m;
 cin >> m;
 vector<Point<R>> qs(m);
 for(auto& p: qs) cin >> p;
 Convex<R> a(ps), b(qs);
 for(const auto& e: b.edges()) {
  a= a.cut(Line<R>(e.p, e.q - e.p), 1);
  if(a.size() == 0) break;
 }
 cout << fixed << setprecision(10) << a.area() << '\n';
 return 0;
}
