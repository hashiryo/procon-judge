// https://yukicoder.me/problems/no/215
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(i64 n, int p, int c);  // 計測区間の外。
//     void run();                   // ここだけ測る。
//     i64 answer() const;           // ゴールする方法の数 mod 1e9+7
//   };
//
// 1 ターンの出目の和の分布 (区別しないサイコロの組合せ) を多項式で作り、有理
// 関数の第 N 項を Bostan-Mori で出す。多項式クラスで組み立てるか、配列の DP と
// 畳み込みで組み立てるかを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-dp-convolve.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  i64 n;
  int p, c;
  must_scan(scanf("%lld %d %d", &n, &p, &c), 3);

  Solver sol(n, p, c);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
