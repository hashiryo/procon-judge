// https://judge.yosupo.jp/problem/subset_convolution
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &a, const vector<i64> &b);
//     void run();                  // 部分集合畳み込み。ここだけ測る。
//     vector<i64> answer() const;  // 2^N 個
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-set-power-series.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> a = read_ints(1 << n);
  vector<i64> b = read_ints(1 << n);

  Solver sol(n, a, b);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
