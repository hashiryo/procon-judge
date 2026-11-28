#include <iostream>
#include <vector>
#include "mylib/geometry/Point.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 int N;
 cin >> N;
 vector<Point<int>> ps(N);
 for(auto& p: ps) cin >> p;
 bool ok= false;
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < i; ++j)
   for(int k= 0; k < j; ++k)
    if(!sgn(cross(ps[i] - ps[j], ps[k] - ps[j]))) ok= true;
 cout << (ok ? "Yes" : "No") << '\n';
 return 0;
}
