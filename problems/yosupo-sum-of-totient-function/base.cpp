// https://judge.yosupo.jp/problem/sum_of_totient_function
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(i64 n);  // 構築。計測区間の外。
//     void run();              // 和を求める。ここだけ測る。
//     i64 answer() const;      // sum phi(i) (i = 1..N) mod 998244353
//   };
//
// 入力は N だけなので、構築に測るものが無い。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-dirichlet-series.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  i64 n;
  must_scan(scanf("%lld", &n), 1);

  Solver sol(n);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
