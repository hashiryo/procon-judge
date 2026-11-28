#include <iostream>
#include "neo/base/core/geo/Segment.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int q;
 cin >> q;
 while(q--) {
  Segment<long double> s, t;
  cin >> s.p >> s.q >> t.p >> t.q;
  cout << (intersection(s, t) != Intersection::disjoint) << '\n';
 }
 return 0;
}
