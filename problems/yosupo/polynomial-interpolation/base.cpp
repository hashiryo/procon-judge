// https://judge.yosupo.jp/problem/polynomial_interpolation
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &x, const vector<i64> &y);  // 構築。計測区間の外。
//     void run();                                         // 補間した多項式を作る。ここだけ測る。
//     vector<i64> coeffs() const;                         // 係数 (n 個)
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。点 x_i は相異なる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-subproduct.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> x = read_ints(n), y = read_ints(n);

  Solver sol(x, y);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.coeffs(), ' ');
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
