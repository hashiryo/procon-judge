#include <iostream>
#include <iomanip>
#include "mylib/geometry/intersection_area.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 Circle<long double> c;
 int n;
 cin >> n >> c.r;
 vector<Point<long double>> ps(n);
 for(int i= 0; i < n; ++i) cin >> ps[i];
 cout << fixed << setprecision(12) << intersection_area(c, Polygon(ps)) << '\n';
 return 0;
}
