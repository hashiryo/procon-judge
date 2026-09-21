// https://yukicoder.me/problems/no/1080
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);              // 計測区間の外。
//     void run();                          // ここだけ測る。
//     const vector<i64> &answer() const;   // K = 1 .. N の答え mod 1e9+9
//   };
//
// スコアの符号 (M mod 4) を虚数単位 i の冪で表すと exp(i g) と exp(-i g) の形になり、
// g の係数 (k+1)^2 は有理関数で書ける。exp を疎な有理関数の形から直接出すか、
// 密な exp と inv で出すかを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-fps-exp.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);

  Solver sol(n);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
