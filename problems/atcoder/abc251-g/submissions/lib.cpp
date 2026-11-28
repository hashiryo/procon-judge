#include <climits>
#include <iostream>
#include <vector>
#include "mylib/geometry/Point.hpp"
using namespace std;
// 各辺 e について、x がすべての平行移動 P + t_i に入る条件は
// cross(d_e, x - p_e) >= max_i cross(d_e, t_i)。この上限を先に M 回で求め、クエリは N 回の比較で答える。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using K= long long;
 int N;
 cin >> N;
 vector<Point<K>> ps(N);
 for(auto& p: ps) cin >> p;
 int M;
 cin >> M;
 vector<Point<K>> ts(M);
 for(auto& t: ts) cin >> t;
 vector<make_long_t<K>> lim(N);
 for(int e= 0; e < N; ++e) {
  Point<K> d= ps[(e + 1) % N] - ps[e];
  lim[e]= -(make_long_t<K>(1) << 120);
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
