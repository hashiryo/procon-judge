// https://onlinejudge.u-aizu.ac.jp/problems/3072
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int k, int p);  // 構築。計測区間の外。
//     void run();                   // ここだけ測る。
//     i64 answer() const;           // 998244353 での値
//   };
//
// 入力は 3 つの整数だけなので、構築に測るものが無い。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-fps-inv.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, k, p;
  must_scan(scanf("%d %d %d", &n, &k, &p), 3);

  Solver sol(n, k, p);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
