// ABC 355 G (Baseball) https://atcoder.jp/contests/abc355/tasks/abc355_g
// lib-neo.cpp と同じ Alien DP を、今の Library の simplified_larsch_dp で解く。
// 値と中心数は 1 語 (__int128) に詰める: 値 * (M+1) + (M - 中心数)。
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#include "mylib/optimization/simplified_larsch_dp.hpp"
using namespace std;
using i64= long long;
using i128= __int128_t;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, K;
 cin >> N >> K;
 vector<i64> pos;
 pos.reserve(100000);
 for(int y= 1; y <= N; ++y) {
  int p;
  cin >> p;
  for(int t= 0; t < p; ++t) pos.push_back(y);
 }
 const int M= pos.size();
 if(M == 0) return cout << 0 << '\n', 0;
 const int Kp= min(K, M);
 vector<i64> sp(M + 1, 0);
 for(int i= 0; i < M; ++i) sp[i + 1]= sp[i] + pos[i];
 auto w= [&](int l, int r) {
  const int c= (l + r) / 2;
  return pos[c] * (c - l) - (sp[c] - sp[l]) + (sp[r + 1] - sp[c + 1]) - pos[c] * (r - c);
 };
 const i128 BASE= M + 1;
 auto solve= [&](i64 lam) -> i128 {
  auto cost= [&](int i, int j) -> i128 { return (i128)(w(j, i - 1) + lam) * BASE - 1; };
  vector<i128> dp= simplified_larsch_dp(M, cost);
  return dp[M] + M;  // 先頭の状態 (値 0、中心数 0) の詰め直しの分
 };
 i64 lo= 0, hi= w(0, M - 1) + 1;
 while(lo + 1 < hi) {
  const i64 mid= (lo + hi) / 2, enc= solve(mid);
  if(M - (i64)(enc % BASE) >= Kp) lo= mid;
  else hi= mid;
 }
 cout << (i64)(solve(lo) / BASE) - lo * Kp << '\n';
 return 0;
}
