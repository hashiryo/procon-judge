#include <iostream>
#include <vector>
#include "neo/base/core/geo/Point.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using K= long long;
 int N;
 cin >> N;
 vector<Point<K>> ps(N);
 for(auto& p: ps) cin >> p;
 int M;
 cin >> M;
 vector<Point<K>> ts(M);
 for(auto& t: ts) cin >> t;
 vector<wide_t<K>> lim(N);
 for(int e= 0; e < N; ++e) {
  Point<K> d= ps[(e + 1) % N] - ps[e];
  lim[e]= -(wide_t<K>(1) << 120);
  for(const auto& t: ts) lim[e]= max(lim[e], cross(d, t));
 }
 int Q;
 cin >> Q;
 while(Q--) {
  Point<K> q;
  cin >> q;
  bool ok= true;
  for(int e= 0; e < N && ok; ++e) {
   Point<K> d= ps[(e + 1) % N] - ps[e];
   ok= cross(d, q - ps[e]) >= lim[e];
  }
  cout << (ok ? "Yes" : "No") << '\n';
 }
 return 0;
}
