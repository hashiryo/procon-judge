// https://yukicoder.me/problems/no/658
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &ns);  // 質問。計測区間の外。
//     void run();                               // ここだけ測る。
//     const vector<i64> &answer() const;        // 質問ごとの T_n mod 17
//   };
//
// テトラナッチ数列の第 n 項 (n <= 1e18) を 17 で割った余り。線形漸化式の第 n 項を
// Bostan-Mori で直接出すか、状態 (直前 4 項) の写像の周期を見つけて飛ぶかを
// 比べる。周期の探索は後者だけにかかる構築なので、計測区間に入れる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-linear-recurrence.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<i64> ns = read_ints(q);

  Solver sol(ns);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
