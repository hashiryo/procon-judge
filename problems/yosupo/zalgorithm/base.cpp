// https://judge.yosupo.jp/problem/zalgorithm
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);   // 構築。計測区間の外。
//     void run();                         // Z 配列を作る。ここだけ測る。
//     const vector<int> &answer() const;  // 結果を取り出す。整形は外。
//   };
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける。
// 内部表現が入出力と違う問題 (ModInt など) でも、この形なら変換が計測区間の
// 外に出る。クエリを順に処理する問題の形とは分けている。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-z-algorithm.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  string s = read_token();

  Solver sol(s);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
