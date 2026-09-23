// https://onlinejudge.u-aizu.ac.jp/courses/library/3/DSL/3/DSL_3_D
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int l, const vector<i64> &a);  // 幅 l の窓
//     void run();                           // 前処理と取得。ここだけ測る。
//     const vector<i64> &answer() const;    // N-L+1 個
//   };
//
// 前処理を run() の中に置く。どちらの実装も表を作ってから引く形なので、
// 表の作り方そのものが比較の対象になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-sparse-table.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, l;
  must_scan(scanf("%d %d", &n, &l), 2);
  vector<i64> a = read_ints(n);

  Solver sol(l, a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
