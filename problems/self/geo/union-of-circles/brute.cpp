#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <tuple>
#include <vector>
#include "neo/base/core/geo/Circle.hpp"
using namespace std;
// 小さい N 用の愚直解。縦の切片の合併の長さを x で積分する。円の境界の交点の x で区切ると、
// 区間の中では合併の上端と下端の円が変わらないので、sqrt の原始関数で厳密に積分できる。
static long double antideriv(const Point<long double>& o, long double r, long double x) {
 long double u= x - o.x, v= max((long double)0, r * r - u * u);
 return u / 2 * sqrt(v) + r * r / 2 * asin(clamp<long double>(u / r, -1, 1));
}
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 int N;
 cin >> N;
 vector<Circle<R>> cs(N);
 for(auto& c: cs) cin >> c.o >> c.r;
 vector<R> xs;
 for(const auto& c: cs) xs.push_back(c.o.x - c.r), xs.push_back(c.o.x + c.r);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < i; ++j)
   for(const auto& p: cross_points(cs[i], cs[j])) xs.push_back(p.x);
 sort(xs.begin(), xs.end());
 R area= 0;
 for(int k= 1; k < (int)xs.size(); ++k) {
  R l= xs[k - 1], r= xs[k];
  if(r - l <= 1e-12L * max((R)1, fabsl(r))) continue;
  R xm= (l + r) / 2;
  vector<tuple<R, R, int>> iv;
  for(int i= 0; i < N; ++i) {
   R u= xm - cs[i].o.x;
   if(fabsl(u) >= cs[i].r) continue;
   R h= sqrt(cs[i].r * cs[i].r - u * u);
   iv.emplace_back(cs[i].o.y - h, cs[i].o.y + h, i);
  }
  if(iv.empty()) continue;
  sort(iv.begin(), iv.end());
  R top= get<1>(iv[0]), bot= get<0>(iv[0]);
  int itop= get<2>(iv[0]), ibot= get<2>(iv[0]);
  auto flush= [&]() {
   area+= (cs[itop].o.y - cs[ibot].o.y) * (r - l) + (antideriv(cs[itop].o, cs[itop].r, r) - antideriv(cs[itop].o, cs[itop].r, l)) + (antideriv(cs[ibot].o, cs[ibot].r, r) - antideriv(cs[ibot].o, cs[ibot].r, l));
  };
  for(int i= 1; i < (int)iv.size(); ++i) {
   auto [lo, hi, id]= iv[i];
   if(lo > top) {
    flush();
    top= hi, bot= lo, itop= id, ibot= id;
   } else if(hi > top) top= hi, itop= id;
  }
  flush();
 }
 cout << fixed << setprecision(10) << area << '\n';
 return 0;
}
