#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/Segment.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 cout << fixed << setprecision(12);
 for(int W, H, N; cin >> W >> H >> N && N;) {
  Segment<R> left({0, 0}, {0, R(H)}), right({R(W), 0}, {R(W), R(H)});
  vector<Point<R>> ps(2 * N);
  for(int i= 0; i < 2 * N; ++i) cin >> ps[i];
  auto check= [&](const Line<R>& l) {
   int on= 0, dif= 0;
   for(auto& p: ps) {
    int w= l.where(p);
    on+= (w == 0), dif+= w;
   }
   return abs(dif) <= on;
  };
  vector<R> xs;
  auto qs= ps;
  qs.emplace_back(right.p), qs.emplace_back(right.q);
  sort(qs.begin(), qs.end()), qs.erase(unique(qs.begin(), qs.end()), qs.end());
  for(int i= qs.size(); i--;)
   for(int j= i; j--;) {
    Line l= line_through(qs[i], qs[j]);
    if(!check(l)) continue;
    if(!cross_points(right, l).size()) continue;
    auto cp= cross_points(left, l);
    if(!cp.size()) continue;
    xs.emplace_back(cp[0].y);
   }
  xs.emplace_back(0), xs.emplace_back(H);
  sort(xs.begin(), xs.end()), xs.erase(unique(xs.begin(), xs.end()), xs.end());
  auto f= [&](R x) {
   R mn= H + 1, mx= -1;
   Point<R> p0= {0, x};
   for(auto& p: qs) {
    if(sgn(p.x) == 0) continue;
    Line l= line_through(p0, p);
    if(!check(l)) continue;
    auto cp= cross_points(right, l);
    if(!cp.size()) continue;
    mn= min(mn, cp[0].y), mx= max(mx, cp[0].y);
   }
   if(mx < 0) return R(0);
   return mx - mn;
  };
  R ans= 0;
  for(int i= xs.size(); --i;) ans+= (xs[i] - xs[i - 1]) * f((xs[i] + xs[i - 1]) / 2);
  ans/= H * H;
  cout << ans << '\n';
 }
 return 0;
}
