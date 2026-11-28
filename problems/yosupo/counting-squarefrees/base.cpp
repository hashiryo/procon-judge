// https://judge.yosupo.jp/problem/counting_squarefrees
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(u64 n);   // 構築。計測区間の外。
//     void run();      // ここだけ測る。
//     u64 answer() const;  // n 以下の無平方数の個数
//   };
//
// 入力は n だけ。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  u64 n;
  must_scan(scanf("%llu", &n), 1);

  Solver sol(n);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%llu\n", sol.answer());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
