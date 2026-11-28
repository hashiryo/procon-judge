#include <iomanip>
#include <iostream>
#include "neo/base/core/geo/Circle.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 Circle<R> c, d;
 cin >> c.o >> c.r >> d.o >> d.r;
 auto ps= cross_points(c, d);
 Point<R> p1= ps[0];
 Point<R> p2= ps.size() > 1 ? ps[1] : ps[0];
 if(p2 < p1) swap(p1, p2);
 cout << fixed << setprecision(12) << p1.x << " " << p1.y << " " << p2.x << " " << p2.y << '\n';
 return 0;
}
