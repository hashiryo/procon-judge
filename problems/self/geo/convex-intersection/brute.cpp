#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/Convex.hpp"
using namespace std;
// 小さい n,m 用の愚直解。2 つの多角形の頂点と、辺どうしの交点のうち、両方の内側か周上にある点の
// 凸包の面積。共通部分の頂点は必ずこのどれかになる。
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
 auto inside= [](const Convex<R>& g, const Point<R>& p) {
  if(g.where(p) >= 0) return true;
  // 距離に直して辺の上に載っているかを見る (係数が大きいと cross の丸め誤差が絶対 EPS を超える)。
  for(const auto& e: g.edges()) {
   Point<R> d= e.q - e.p;
   if(std::abs((double)(cross(d, p - e.p) / std::sqrt((long double)norm2(d)))) > 1e-9) continue;
   double t= (double)(dot(d, p - e.p) / norm2(d));
   if(-1e-9 <= t && t <= 1 + 1e-9) return true;
  }
  return false;
 };
 vector<Point<R>> cand(ps.begin(), ps.end());
 cand.insert(cand.end(), qs.begin(), qs.end());
 for(const auto& e: a.edges())
  for(const auto& f: b.edges()) {
   auto cp= cross_points(e, f);
   cand.insert(cand.end(), cp.begin(), cp.end());
  }
 vector<Point<R>> ok;
 for(const auto& p: cand)
  if(inside(a, p) && inside(b, p)) ok.push_back(p);
 cout << fixed << setprecision(10) << (ok.size() < 3 ? R(0) : Convex<R>(ok).area()) << '\n';
 return 0;
}
