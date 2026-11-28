#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/closest_pair.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n;
 cin >> n;
 vector<Point<long double>> ps(n);
 for(auto& p: ps) cin >> p;
 auto [p, q]= closest_pair(ps);
 cout << fixed << setprecision(12) << dist(p, q) << '\n';
 return 0;
}
