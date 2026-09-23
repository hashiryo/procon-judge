// https://onlinejudge.u-aizu.ac.jp/challenges/sources/JOI/Final/0661
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 2>> &pts);  // 2n 個の点
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 点を n x 2 の格子へ 1 つずつ詰めるときの総移動距離を最小にする。格子の
// 外に出ている点を中へ寄せる費用と、各マスに何個来たかを数えるところは
// どちらの実装でも同じなので common.hpp に置いた。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-primal.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<array<i64, 2>> pts(2 * n);
  for (auto &p : pts) must_scan(scanf("%lld %lld", &p[0], &p[1]), 2);

  Solver sol(n, pts);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
