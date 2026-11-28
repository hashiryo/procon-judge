#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/Convex.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 cout << fixed << setprecision(12);
 using P= Point<long double>;
 int n;
 cin >> n;
 vector<P> ps(n);
 for(auto& p: ps) cin >> p;
 Convex<long double> g(ps);
 int q;
 cin >> q;
 while(q--) {
  P lp, lq;
  cin >> lp >> lq;
  cout << g.cut(line_through(lp, lq)).area() << '\n';
 }
 return 0;
}
