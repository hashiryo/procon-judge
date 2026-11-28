#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/intersection_area.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 Circle<long double> c;
 int n;
 cin >> n >> c.r;
 vector<Point<long double>> ps(n);
 for(auto& p: ps) cin >> p;
 cout << fixed << setprecision(12) << intersection_area(c, Polygon<long double>(ps)) << '\n';
 return 0;
}
