#include <iostream>
#include <iomanip>
#include "mylib/geometry/Polygon.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 int n;
 cin >> n;
 vector<Point<long double>> ps(n);
 for(int i= 0; i < n; ++i) cin >> ps[i];
 Polygon g(ps);
 cout << fixed << setprecision(12) << g.area() << '\n';
 return 0;
}
