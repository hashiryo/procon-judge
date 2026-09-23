// https://yukicoder.me/problems/no/1595
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(i64 p, i64 q, i64 r, i64 k);  // 計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;  // A_K の 1 の位
//   };
//
// トリボナッチ型の数列の第 K 項 (K <= 1e18) の 1 の位。線形漸化式の第 K 項を
// Bostan-Mori で直接出すか、状態 (直前 3 項の 1 の位) の写像の周期を見つけて飛ぶ
// かを比べる。周期の探索は後者だけにかかる構築なので、計測区間に入れる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-linear-recurrence.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  i64 p, q, r, k;
  must_scan(scanf("%lld %lld %lld %lld", &p, &q, &r, &k), 4);

  Solver sol(p, q, r, k);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
