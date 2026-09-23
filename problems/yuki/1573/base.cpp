// https://yukicoder.me/problems/no/1573
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(i64 n, i64 m);  // 計測区間の外。
//     void run();            // ここだけ測る。
//     i64 answer() const;    // mod 998244353
//   };
//
// sum_{i<=n} f(i, m) を約数の和の形に整理して数える。Dirichlet 級数の積として
// O(N^(2/3)) で出すか、商で区切った区間ごとに O(sqrt N) で足すかを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-enumerate-quotients.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  i64 n, m;
  must_scan(scanf("%lld %lld", &n, &m), 2);

  Solver sol(n, m);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
