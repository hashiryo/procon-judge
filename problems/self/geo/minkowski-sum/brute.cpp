#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/algo/compute/geo/half_plane_intersection.hpp"
using namespace std;
// 小さい n,m 用の愚直解。Minkowski 和は、a と b のすべての辺の外向き法線 n について
// n . x <= h_a(n) + h_b(n) を満たす点の集合。この半平面の共通部分を半平面交差で作る。
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
 Convex<R> a(ps), b(qs);
 auto support= [](const Convex<R>& c, const Point<R>& u) {
  R h= dot(u, c[0]);
  for(const auto& p: c) h= max(h, dot(u, p));
  return h;
 };
 vector<Line<R>> ls;
 auto add_edges= [&](const Convex<R>& c) {
  for(const auto& e: c.edges()) {
   if(!sgn(norm2(e.q - e.p))) continue;  // 1 点の多角形
   Point<R> u(e.q.y - e.p.y, -(e.q.x - e.p.x));  // 外向き法線 (u.x, u.y) = (dy, -dx)
   R h= support(a, u) + support(b, u);
   // u . x <= h  <=>  -u.x x - u.y y + h >= 0
   ls.emplace_back(-u.x, -u.y, h);
  }
 };
 add_edges(a), add_edges(b);
 cout << fixed << setprecision(10) << half_plane_intersection(ls).area() << '\n';
 return 0;
}
