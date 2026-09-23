#include <iostream>
#include <iomanip>
#include "mylib/geometry/Segment.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 cout << fixed << setprecision(12);
 int q;
 cin >> q;
 while(q--) {
  Segment<long double> s, t;
  cin >> s.p >> s.q >> t.p >> t.q;
  cout << dist(s, t) << '\n';
 }
 return 0;
}
