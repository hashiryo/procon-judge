#include <iostream>
#include "neo/base/core/geo/Circle.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 Circle<long double> c, d;
 cin >> c.o >> c.r >> d.o >> d.r;
 cout << common_tangent(c, d).size() << '\n';
 return 0;
}
