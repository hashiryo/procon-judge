#include <iostream>
#include "mylib/geometry/Circle.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 Circle<long double> c, d;
 cin >> c.o >> c.r;
 cin >> d.o >> d.r;
 cout << common_tangent(c, d).size() << '\n';
 return 0;
}
