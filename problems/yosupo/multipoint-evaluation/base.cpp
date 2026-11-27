// https://judge.yosupo.jp/problem/multipoint_evaluation
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &f, const vector<i64> &x);  // 構築。計測区間の外。
//     void run();                                         // f(x_i) を作る。ここだけ測る。
//     vector<i64> values() const;                         // f(x_0), …, f(x_{m-1})
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。内部表現を提出が選べるようにするためで、
// 変換は O(N)、計算は O(N log N) なので、一緒に測ると表現の選び方で乗り方が変わる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-subproduct.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<i64> f = read_ints(n), x = read_ints(m);

  Solver sol(f, x);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.values(), ' ');
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
