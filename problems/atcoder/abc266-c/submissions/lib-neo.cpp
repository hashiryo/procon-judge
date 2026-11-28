#include <iostream>
#include "neo/base/core/geo/Point.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using P= Point<int>;
 P p[4];
 for(auto& q: p) cin >> q;
 bool ok= true;
 for(int i= 0; i < 4; ++i)
  if(ccw(p[i], p[(i + 1) % 4], p[(i + 2) % 4]) != Ccw::counter_clockwise) ok= false;
 cout << (ok ? "Yes" : "No") << '\n';
 return 0;
}
