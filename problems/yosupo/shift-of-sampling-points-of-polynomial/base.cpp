// https://judge.yosupo.jp/problem/shift_of_sampling_points_of_polynomial
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &y, i64 c, int m);  // 構築。計測区間の外。
//     void run();                                 // f(c), …, f(c + m - 1) を作る。ここだけ測る。
//     vector<i64> values() const;                 // m 個
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。y_i = f(i) (0 <= i < n)、f は n - 1 次以下。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-sample-points.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  i64 c;
  must_scan(scanf("%d %d %lld", &n, &m, &c), 3);
  vector<i64> y = read_ints(n);

  Solver sol(y, c, m);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.values(), ' ');
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
