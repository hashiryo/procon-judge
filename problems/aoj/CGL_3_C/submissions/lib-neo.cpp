#include <iostream>
#include <vector>
#include "neo/base/core/geo/Polygon.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using P= Point<int>;
 int n;
 cin >> n;
 vector<P> ps(n);
 for(auto& p: ps) cin >> p;
 Polygon<int> g(ps);
 int q;
 cin >> q;
 while(q--) {
  P p;
  cin >> p;
  cout << g.where(p) + 1 << '\n';
 }
 return 0;
}
