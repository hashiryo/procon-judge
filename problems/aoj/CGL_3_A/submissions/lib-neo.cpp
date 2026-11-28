#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/Polygon.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n;
 cin >> n;
 vector<Point<long double>> ps(n);
 for(auto& p: ps) cin >> p;
 Polygon<long double> g(ps);
 cout << fixed << setprecision(12) << g.area() << '\n';
 return 0;
}
