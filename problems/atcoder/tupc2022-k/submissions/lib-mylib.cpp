// TUPC 2022 K (Lebesgue Integral) https://atcoder.jp/contests/tupc2022/tasks/tupc2022-k
// lib-neo.cpp と同じ Alien DP を、今の Library の ConvexHullTrick (multiset、傾きの順を問わない) で解く。
// 値と区間数は 1 語に詰める (値 * (N+1) + (N - 区間数))。
#include <algorithm>
#include <iostream>
#include <vector>
#include "mylib/optimization/ConvexHullTrick.hpp"
using namespace std;
using i64= long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, K;
 cin >> N >> K;
 vector<i64> A(N);
 for(auto& x: A) cin >> x;
 sort(A.begin(), A.end());
 const int Kp= min(K, N);
 const i64 BASE= N + 1;
 auto solve= [&](i64 lam) -> i64 {
  ConvexHullTrick<i64, MINIMIZE> cht;
  i64 g= N;  // j = 0: 値 0、区間数 0
  cht.insert(0, g);
  for(int i= 1; i <= N; ++i) {
   const i64 x= A[i - 1];
   g= cht.query(x) + (i64)i * x * BASE + lam * BASE - 1;
   cht.insert(-(i64)i * BASE, g);
  }
  return g;
 };
 i64 lo= 0, hi= A.back() * N + 1;
 while(lo + 1 < hi) {
  const i64 mid= (lo + hi) / 2, enc= solve(mid);
  if(N - (enc % BASE) >= Kp) lo= mid;
  else hi= mid;
 }
 cout << solve(lo) / BASE - lo * Kp << '\n';
 return 0;
}
