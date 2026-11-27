// https://judge.yosupo.jp/problem/composition_of_formal_power_series
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &f, const vector<i64> &g);  // 構築。計測区間の外。
//     void run();                                         // f(g) mod x^n を作る。ここだけ測る。
//     vector<i64> coeffs() const;                         // n 項の係数
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。内部表現を提出が選べるようにする
// ためで、理由はハーネスの節に書いてある。変換は O(N)、計算は O(N log N) なので、
// 一緒に測ると表現の選び方で乗り方が変わる。g の定数項は 0 で渡される。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-polynomial.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> f = read_ints(n), g = read_ints(n);

  Solver sol(f, g);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.coeffs(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
