#include <iostream>
#include <vector>
#include "neo/base/core/geo/Polygon.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n;
 cin >> n;
 vector<Point<int>> ps(n);
 for(auto& p: ps) cin >> p;
 Polygon<int> g(ps);
 cout << g.is_convex() << '\n';
 return 0;
}
