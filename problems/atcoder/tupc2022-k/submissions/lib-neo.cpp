// TUPC 2022 K (Lebesgue Integral) https://atcoder.jp/contests/tupc2022/tasks/tupc2022_k
// A をソートし、連続する区間に分けて Σ (区間の最大) × (区間の長さ) を最小にする。
// Alien DP: 1 区間あたり λ を足した DP を解き、区間数が K 以上になる最大の λ を二分探索する。
// 内側の DP は傾き -j、問い A_i がどちらも単調なので、NeoLibrary の MonotoneCHT (両端キュー) で O(N)。
// 値と区間数は 1 語に詰める (値 * (N+1) + (N - 区間数))。同点は区間数が多いほうが勝つので、
// Alien DP の区間数の単調性がそのまま使える。
#include <algorithm>
#include <iostream>
#include <vector>
#include "neo/algo/optimize/line/MonotoneCHT.hpp"
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
 // λ を足した DP。戻り値は詰めた値。
 auto solve= [&](i64 lam) -> i64 {
  MonotoneCHT<i64> cht;
  cht.reserve(N + 1);
  i64 g= N;  // j = 0: 値 0、区間数 0
  cht.add_line(0, g);
  for(int i= 1; i <= N; ++i) {
   const i64 x= A[i - 1];
   g= cht.query_monotone(x) + (i64)i * x * BASE + lam * BASE - 1;
   cht.add_line(-(i64)i * BASE, g);
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
