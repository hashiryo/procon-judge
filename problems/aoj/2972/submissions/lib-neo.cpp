#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/Point.hpp"
using namespace std;
// 黄金分割探索。[l, r] で f を最小にする点の f の値を返す。
template <class F> long double golden_min(const F& f, long double l, long double r, int iter= 100) {
 static constexpr long double c= 0.38196601125;
 long double x= l + (r - l) * c, y= r - (r - l) * c;
 long double fx= f(x), fy= f(y);
 for(bool g; iter--;) {
  g= fx < fy;
  if(g) r= y, y= x, fy= fx, fx= f(x= l + (r - l) * c);
  else l= x, x= y, fx= fy, fy= f(y= r - (r - l) * c);
 }
 return fx;
}
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 int N, K;
 cin >> N >> K;
 vector<Point<R>> ps(N);
 for(int i= 0; i < N; ++i) cin >> ps[i];
 auto score= [&](const Point<R>& p) {
  vector<R> dis(N);
  for(int i= 0; i < N; ++i) dis[i]= dist(ps[i], p);
  sort(dis.rbegin(), dis.rend());
  R ret= 0;
  for(int i= 0; i < K; ++i) ret+= dis[i];
  return ret;
 };
 auto f= [&](R x) {
  auto g= [&](R y) { return score({x, y}); };
  return golden_min(g, -1000.0, 1000.0);
 };
 cout << fixed << setprecision(10) << golden_min(f, -1000.0, 1000.0) << '\n';
 return 0;
}
