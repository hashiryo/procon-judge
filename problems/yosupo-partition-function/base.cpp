// https://judge.yosupo.jp/problem/partition_function
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);      // 構築。計測区間の外。
//     void run();                  // 分割数を作る。ここだけ測る。
//     vector<i64> answer() const;  // p(0) から p(N) の N+1 個
//   };
//
// 剰余は 998244353 で、出力は素の整数で受け取る。入力は N だけなので、構築に
// 測るものが無い。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-sequences.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);

  Solver sol(n);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
