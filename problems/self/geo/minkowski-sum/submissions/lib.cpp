#include <iomanip>
#include <iostream>
#include <vector>
#include "mylib/geometry/Convex.hpp"
using namespace std;
// 頂点の全ペアの和の凸包。O(n * m の点の凸包)。
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
 vector<Point<R>> sums;
 sums.reserve(a.size() * b.size());
 for(const auto& p: a)
  for(const auto& q: b) sums.push_back(p + q);
 cout << fixed << setprecision(10) << Convex<R>(sums).area() << '\n';
 return 0;
}
