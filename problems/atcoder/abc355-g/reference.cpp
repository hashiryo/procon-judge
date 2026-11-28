// ABC 355 G (Baseball) https://atcoder.jp/contests/abc355/tasks/abc355_g
//
// 1..N から K 個の相異なる位置に中心を置き、Σ_y P_y min_i |x_i - y| を最小にする。ΣP ≤ 1e5 なので
// 位置 y を P_y 個に展開し (M = ΣP ≤ 1e5)、ソート済みの点列を連続する区間に分ける問題にする。
// 区間の費用 w(l, r) は中央値の点で受けるときの距離の和で、累積和で O(1)。
// K が M まで大きいので Alien DP: 中心 1 個あたり λ を足した DP を解き、中心数が K 以上になる
// 最大の λ を二分探索する。内側の DP は同じ層の dp を参照するので、LARSCH (simplified) で O(M log M)。
// 値と中心数は 1 語に詰める (値 * (M+1) + (M - 中心数) を __int128 で)。中心数 ≤ 1e5、
// 値 ≤ 5e9 + 5e9 * 1e5 なので 64 bit には収まらない。
// 期待出力を作る側なので、ライブラリを include しない。
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;
using i64= long long;
using i128= __int128_t;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, K;
 if(!(cin >> N >> K)) return 0;
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
 // w(l, r) = 点 l..r を 1 つの中心で受ける費用。中心は中央値の点に置く。
 auto w= [&](int l, int r) {
  const int c= (l + r) / 2;
  return pos[c] * (c - l) - (sp[c] - sp[l]) + (sp[r + 1] - sp[c + 1]) - pos[c] * (r - c);
 };
 const i128 BASE= M + 1;
 // λ を足した DP。戻り値は詰めた値。
 auto solve= [&](i64 lam) -> i128 {
  // LARSCH (simplified)。w が Monge なので、行 m の最良の列は行 l, r の最良の列の間に絞れる。
  vector<i128> dp(M + 1, (i128)1 << 120);
  vector<int> best(M + 1);
  dp[0]= M;  // 値 0、中心数 0
  auto relax= [&](int i, int j) {
   const i128 v= dp[j] + (i128)(w(j, i - 1) + lam) * BASE - 1;
   if(v < dp[i]) dp[i]= v, best[i]= j;
  };
  relax(M, 0);
  auto rec= [&](auto& rec, int l, int r) -> void {
   if(r - l <= 1) return;
   const int m= (l + r) / 2;
   for(int j= best[l]; j <= best[r]; ++j) relax(m, j);
   rec(rec, l, m);
   for(int j= l + 1; j <= m; ++j) relax(r, j);
   rec(rec, m, r);
  };
  rec(rec, 0, M);
  return dp[M];
 };
 // λ = 0 では中心数 M、λ ≥ 全体を 1 中心で受ける費用 + 1 では 1 なので、Kp 以上になる最大の λ を探す。
 i64 lo= 0, hi= w(0, M - 1) + 1;
 while(lo + 1 < hi) {
  const i64 mid= (lo + hi) / 2, enc= solve(mid);
  if(M - (i64)(enc % BASE) >= Kp) lo= mid;
  else hi= mid;
 }
 const i128 enc= solve(lo);
 cout << (i64)(enc / BASE) - lo * Kp << '\n';
 return 0;
}
