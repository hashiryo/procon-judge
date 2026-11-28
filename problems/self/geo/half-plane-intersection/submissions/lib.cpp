#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "mylib/geometry/Convex.hpp"
using namespace std;
// 箱から始めて、半平面を 1 本ずつで切る。O(N * 多角形の頂点数)。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using R= long double;
 using P= Point<R>;
 int N;
 cin >> N;
 constexpr R B= 200000;
 Convex<R> c({P(-B, -B), P(B, -B), P(B, B), P(-B, B)});
 for(int i= 0; i < N; ++i) {
  R a, b, d;
  cin >> a >> b >> d;
  c= c.cut(Line<R>(a, b, d), 1);
 }
 cout << fixed << setprecision(10) << c.area() << '\n';
 return 0;
}
