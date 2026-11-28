#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/Convex.hpp"
using namespace std;
// 小さい N 用の愚直解。すべての直線の交点のうち、すべての半平面を満たす点の凸包の面積。
// 頂点は必ず 2 本の境界線の交点なので、これで共通部分の多角形ができる。O(N^3)。
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
 vector<Point<R>> ps;
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < i; ++j) {
   auto cp= cross_points(ls[i], ls[j]);
   if(cp.size() != 1) continue;
   bool ok= true;
   for(const auto& l: ls) {
    // 距離に直してから見る (係数が大きいと cross の丸め誤差が 1e-9 を超える)。
    if(cross(l.d, cp[0] - l.p) < -1e-9 * std::sqrt((long double)norm2(l.d))) {
     ok= false;
     break;
    }
   }
   if(ok) ps.push_back(cp[0]);
  }
 cout << fixed << setprecision(10) << (ps.size() < 3 ? R(0) : Convex<R>(ps).area()) << '\n';
 return 0;
}
