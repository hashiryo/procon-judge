// ABC 355 G (Baseball) https://atcoder.jp/contests/abc355/tasks/abc355_g
// 位置を P_y 個に展開し (M = ΣP ≤ 1e5)、ソート済みの点列を連続する区間に分ける 1 次元 k-median にする。
// Alien DP: 中心 1 個あたり λ を足した DP を解き、中心数が K 以上になる最大の λ を二分探索する。
// 内側の DP は同じ層の dp を参照するので、NeoLibrary の larsch_dp (simplified LARSCH) で O(M log M)。
// 値と中心数は 1 語 (__int128) に詰める: 値 * (M+1) + (M - 中心数)。同点は中心数が多いほうが勝つ。
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#include "neo/algo/optimize/monge/larsch.hpp"
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
 // λ を足した DP。戻り値は詰めた値。
 auto solve= [&](i64 lam) -> i128 {
  auto cost= [&](int i, int j) -> i128 { return (i128)(w(j, i - 1) + lam) * BASE - 1; };
  // larsch_dp は dp[0] = 0 から始めるので、j = 0 の状態 (値 0、中心数 0) を M だけ足して詰め直す。
  vector<i128> dp= larsch_dp<i128>(M, cost);
  return dp[M] + M;
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
