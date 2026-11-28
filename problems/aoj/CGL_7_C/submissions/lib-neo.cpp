#include <iomanip>
#include <iostream>
#include "neo/base/core/geo/Circle.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 Point<long double> A, B, C;
 cin >> A >> B >> C;
 auto c= circumscribed_circle(A, B, C);
 cout << fixed << setprecision(12) << c.o.x << " " << c.o.y << " " << c.r << '\n';
 return 0;
}
