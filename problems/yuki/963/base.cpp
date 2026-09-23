// https://yukicoder.me/problems/no/963
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);  // 計測区間の外。
//     void run();              // ここだけ測る。
//     i64 answer() const;      // 門松列列の数 mod 1012924417
//   };
//
// 交代順列 (up-down permutation) の数の 2 倍。指数型母関数 f が f' = (f^2 + 1) / 2
// を満たすので、形式的冪級数をその不動点として定義して係数を読むか、数列を
// 直接生成する関数で出すかを比べる。
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

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
