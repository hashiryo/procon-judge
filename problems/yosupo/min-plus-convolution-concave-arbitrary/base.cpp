// https://judge.yosupo.jp/problem/min_plus_convolution_concave_arbitrary
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &a, const vector<i64> &b);  // a が凹
//     void run();                                          // ここだけ測る。
//     const vector<i64> &answer() const;                   // N+M-1 個
//   };
//
// c[k] = min over i+j=k の a[i] + b[j]。a が凹だと、行 k 列 j の行列は反 Monge になり、
// 有効な列の階段 (j <= k かつ k-j < N) の上では行最小の列が行について単調にならない。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo-monge.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<i64> a = read_ints(n);
  vector<i64> b = read_ints(m);

  Solver sol(a, b);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
