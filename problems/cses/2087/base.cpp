// https://cses.fi/problemset/task/2087
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &c, int k);  // 構築。計測区間の外。
//     void run();            // ここだけ測る。
//     i64 answer() const;    // 最小の合計距離
//   };
//
// 直線上の n 軒 (位置 1..n) に c_i 人の子がいる。k 校を家に置き、子は近い方の学校へ歩く。
// 学校の受け持ちは連続した区間になるので、区間 [l, r] に学校 1 校を置く費用 w(l, r) を
// 子の数の重み付き中央値で作り、dp[t][i] = min_{j < i} dp[t-1][j] + w(j+1, i) を解く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, k;
  must_scan(scanf("%d %d", &n, &k), 2);
  vector<i64> c = read_ints(n);

  Solver sol(c, k);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", (long long)sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
