#include <iomanip>
#include <iostream>
#include "neo/base/core/geo/Segment.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 cout << fixed << setprecision(12);
 int q;
 cin >> q;
 while(q--) {
  Segment<long double> s, t;
  cin >> s.p >> s.q >> t.p >> t.q;
  auto ans= cross_points(s, t)[0];
  cout << ans.x << " " << ans.y << '\n';
 }
 return 0;
}
