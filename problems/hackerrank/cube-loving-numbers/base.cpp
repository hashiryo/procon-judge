// https://www.hackerrank.com/contests/university-codesprint-5/challenges/cube-loving-numbers
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &ns);  // テストケースごとの N。計測区間の外。
//     void run();                               // ここだけ測る。
//     const vector<i64> &answer() const;        // N 以下の cube-loving number の数
//   };
//
// 2 以上の立方数で割り切れる数を数える。包除で sum_{x>=2} -mu(x) floor(N / x^3)。
// メビウス関数の表を引いて直接足すか、floor(N / a^3) の表に倍数メビウス変換を
// かけて「ちょうど a^3 が最大の立方因子」の数を出して足すかを比べる。テストケースは
// 全部まとめて渡す。テストデータは HackerRank から手で取り込む (source = manual)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-mobius-table.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<i64> ns = read_ints(t);

  Solver sol(ns);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
