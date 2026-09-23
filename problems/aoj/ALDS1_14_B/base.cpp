// https://onlinejudge.u-aizu.ac.jp/courses/lesson/1/ALDS1/14/ALDS1_14_B
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const string &t, const string &p);  // 構築。計測区間の外。
//     void run();                                // 探索。ここだけ測る。
//     const vector<int> &answer() const;         // 出現位置を昇順に
//   };
//
// 文字列を持つだけなので構築に測るものが無い。前処理の作り方が実装ごとに
// 違うので、そこは run() の中に入る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-z-algorithm.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  string t = read_token();
  string p = read_token();

  Solver sol(t, p);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
