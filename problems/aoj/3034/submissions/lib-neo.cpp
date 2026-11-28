#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/algo/compute/geo/min_enclosing_circle.hpp"
#include "neo/base/core/geo/Circle.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using R= long double;
 // 支援集合から最小包含円の半径を作る。
 auto mec_r= [](const vector<Point<R>>& qs) {
  auto s= min_enclosing_circle(qs);
  if(s.m <= 1) return R(0);
  if(s.m == 2) return R(dist(s.a, s.b) / 2);
  return circumscribed_circle(s.a, s.b, s.c).r;
 };
 int N, M;
 cin >> N >> M;
 vector<Point<R>> ps(N);
 for(int i= 0; i < N; ++i) cin >> ps[i];
 int all= 1 << N;
 R min_r[all];
 for(int S= all; --S;) {
  vector<Point<R>> qs;
  for(int i= N; i--;)
   if((S >> i) & 1) qs.push_back(ps[i]);
  min_r[S]= mec_r(qs);
 }
 R dp[all];
 fill_n(dp, all, 1e10), dp[0]= 0;
 for(int j= M; j--;)
  for(int S= all; --S;)
   for(int T= S; T; --T&= S) dp[S]= min(dp[S], max(dp[S ^ T], min_r[T]));
 cout << fixed << setprecision(12) << dp[all - 1] << '\n';
 return 0;
}
