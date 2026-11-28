#include <iostream>
#include "mylib/geometry/Point.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using P= Point<int>;
 P p[4];
 for(auto& q: p) cin >> q;
 bool ok= true;
 for(int i= 0; i < 4; ++i)
  if(ccw(p[i], p[(i + 1) % 4], p[(i + 2) % 4]) != COUNTER_CLOCKWISE) ok= false;
 cout << (ok ? "Yes" : "No") << '\n';
 return 0;
}
