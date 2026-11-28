#include <iomanip>
#include <iostream>
#include <numbers>
#include <vector>
#include "neo/base/core/geo/Circle.hpp"
#include "neo/base/core/geo/Polygon.hpp"
#include "neo/base/core/geo/angle.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Real= long double;
 cout << fixed << setprecision(12);
 for(int N, R, Q; cin >> N >> R >> Q && N;) {
  vector<Point<Real>> ps(N);
  for(auto& p: ps) cin >> p;
  Polygon<Real> g(ps);
  int cur= -1;
  for(int i= N; i--;)
   if(!sgn(norm2(g[i]) - R * R)) cur= i;
  Circle<Real> c({0, 0}, R);
  while(Q--) {
   Real mn= std::numbers::pi_v<Real> * 2;
   int nxt= -1;
   for(int i= N; i--;) {
    if(cur == i) continue;
    Circle<Real> d(g[cur], dist(g[cur], g[i]));
    auto cp= cross_points(c, d);
    auto th= angle(g[i] - g[cur], cp[0] - g[cur]);
    if(mn > th) mn= th, nxt= i;
   }
   g= rotate(g[cur], mn)(g);
   cur= nxt;
   cout << g[cur].x << " " << g[cur].y << '\n';
  }
 }
 return 0;
}
