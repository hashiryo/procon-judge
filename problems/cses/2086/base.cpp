// https://cses.fi/problemset/task/2086
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &a, int k);
//     void run();            // ここだけ測る。
//     i64 answer() const;    // 最小の費用
//   };
//
// 配列を k 個の空でない連続した区間に分け、各区間の和の 2 乗の和を最小にする。
// 費用 (S_i - S_j)^2 は、S が広義単調増加なので Monge になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, k;
  must_scan(scanf("%d %d", &n, &k), 2);
  vector<i64> a = read_ints(n);

  Solver sol(a, k);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", (long long)sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
