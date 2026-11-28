#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <utility>
#include <vector>
#include "mylib/geometry/Circle.hpp"
using namespace std;
// 各円の周のうち他のどの円の内側にも入らない弧を集め、Green の定理で面積にする。O(N^2 log N)。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using R= long double;
 int N;
 cin >> N;
 vector<Circle<R>> cs(N);
 for(auto& c: cs) cin >> c.o >> c.r;
 const R tau= 2 * numbers::pi_v<R>;
 sort(cs.begin(), cs.end(), [](const Circle<R>& a, const Circle<R>& b) {
  if(a.o.x != b.o.x) return a.o.x < b.o.x;
  if(a.o.y != b.o.y) return a.o.y < b.o.y;
  return a.r < b.r;
 });
 cs.erase(unique(cs.begin(), cs.end(), [](const Circle<R>& a, const Circle<R>& b) { return a.o == b.o && a.r == b.r; }), cs.end());
 auto arc_area= [](const Circle<R>& c, R a, R b) {
  return (c.r * c.r * (b - a) + c.r * (c.o.x * (sin(b) - sin(a)) - c.o.y * (cos(b) - cos(a)))) / 2;
 };
 R ans= 0;
 for(int i= 0; i < (int)cs.size(); ++i) {
  const R ri= cs[i].r;
  vector<pair<R, R>> iv;
  bool covered= false;
  for(int j= 0; j < (int)cs.size(); ++j) {
   if(i == j) continue;
   const R rj= cs[j].r, d= dist(cs[i].o, cs[j].o);
   if(sgn(d + ri - rj) <= 0) {
    covered= true;
    break;
   }
   if(sgn(d + rj - ri) <= 0 || sgn(d - (ri + rj)) >= 0) continue;
   R c= (ri * ri + d * d - rj * rj) / (2 * ri * d);
   c= clamp(c, R(-1), R(1));
   R a= atan2(cs[j].o.y - cs[i].o.y, cs[j].o.x - cs[i].o.x) - acos(c);
   R b= a + 2 * acos(c);
   a= fmod(a, tau), a+= a < 0 ? tau : 0;
   b= fmod(b, tau), b+= b < 0 ? tau : 0;
   if(a <= b) iv.emplace_back(a, b);
   else iv.emplace_back(a, tau), iv.emplace_back(0, b);
  }
  if(covered || iv.empty()) {
   if(!covered) ans+= numbers::pi_v<R> * ri * ri;
   continue;
  }
  sort(iv.begin(), iv.end());
  R prev= 0;
  for(auto [a, b]: iv) {
   if(b <= prev) continue;
   if(a > prev) ans+= arc_area(cs[i], prev, a);
   prev= max(prev, b);
  }
  if(prev < tau) ans+= arc_area(cs[i], prev, tau);
 }
 cout << fixed << setprecision(10) << ans << '\n';
 return 0;
}
