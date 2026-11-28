#include <algorithm>
#include <iomanip>
#include <iostream>
#include "neo/base/core/geo/Polygon.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 cout << fixed << setprecision(12);
 for(int N; cin >> N && N;) {
  Segment<R> s;
  cin >> s.p >> s.q;
  R ans= 1e10;
  for(int i= 0; i < N; i++) {
   R l, d, r, u, h;
   cin >> l >> d >> r >> u >> h;
   Polygon<R> g({{l, d}, {r, d}, {r, u}, {l, u}});
   R dis= sqrt((long double)dist2(g, s));
   if(dis <= h) ans= min(ans, dis);
   else ans= min(ans, (dis * dis + h * h) / (2 * h));
  }
  cout << ans << '\n';
 }
 return 0;
}
