#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/Convex.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 cout << fixed << setprecision(10);
 using R= long double;
 for(int n, d, V; cin >> n >> d >> V && n;) {
  vector<Point<R>> ps(n);
  for(int i= 0; i < n; ++i) cin >> ps[i];
  Convex ch(ps);
  R ans= 0;
  for(const auto& e: ch.edges()) {
   Point<R> v= e.q - e.p, u= !v / sqrt((long double)norm2(v));
   R l= 0, h= 3000;
   for(int cnt= 100; cnt--;) {
    R m= (l + h) / 2;
    R a= ch.cut(Line(e.p + m * u, v), -1).area();
    if(a * d < V) l= m;
    else h= m;
   }
   ans= max(ans, l);
  }
  cout << ans << '\n';
 }
 return 0;
}
