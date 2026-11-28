// https://cses.fi/problemset/task/2088
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &x);  // 構築。計測区間の外。
//     void run();            // ここだけ測る。
//     i64 answer() const;    // 最小の合計費用
//   };
//
// 配列を 1 要素ずつの部分配列に分ける。1 回の分割は、選んだ部分配列の和の費用がかかる。
// dp[l][r] = min_{l <= k < r} dp[l][k] + dp[k+1][r] + sum(l, r)、答えは dp[0][n-1]。
// 費用の区間和は Monge (等号) かつ単調なので、Knuth の最適化が使える。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> x = read_ints(n);

  Solver sol(x);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", (long long)sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
