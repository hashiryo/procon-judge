#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/algo/compute/geo/half_plane_intersection.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 int N;
 cin >> N;
 vector<Line<R>> ls(N);
 for(auto& l: ls) {
  R a, b, c;
  cin >> a >> b >> c;
  l= Line<R>(a, b, c);
 }
 cout << fixed << setprecision(10) << half_plane_intersection(ls).area() << '\n';
 return 0;
}
