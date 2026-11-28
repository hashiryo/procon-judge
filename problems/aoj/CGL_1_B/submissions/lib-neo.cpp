#include <iomanip>
#include <iostream>
#include "neo/base/core/geo/Line.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 cout << fixed << setprecision(12);
 using P= Point<long double>;
 P p1, p2;
 cin >> p1 >> p2;
 auto ref= reflect(line_through(p1, p2));
 int q;
 cin >> q;
 while(q--) {
  P p;
  cin >> p;
  auto ans= ref(p);
  cout << ans.x << " " << ans.y << '\n';
 }
 return 0;
}
