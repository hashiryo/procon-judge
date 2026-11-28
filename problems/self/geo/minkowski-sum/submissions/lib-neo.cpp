#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/algo/compute/geo/minkowski_sum.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 int n;
 cin >> n;
 vector<Point<R>> ps(n);
 for(auto& p: ps) cin >> p;
 int m;
 cin >> m;
 vector<Point<R>> qs(m);
 for(auto& p: qs) cin >> p;
 cout << fixed << setprecision(10) << minkowski_sum(Convex<R>(ps), Convex<R>(qs)).area() << '\n';
 return 0;
}
