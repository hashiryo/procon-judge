#include <iomanip>
#include <iostream>
#include "neo/base/core/geo/intersection_area.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 Circle<long double> c, d;
 cin >> c.o >> c.r >> d.o >> d.r;
 cout << fixed << setprecision(12) << intersection_area(c, d) << '\n';
 return 0;
}
