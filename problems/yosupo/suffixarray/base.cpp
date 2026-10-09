// https://judge.yosupo.jp/problem/suffixarray
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);   // 構築。文字列を写すだけで、計測区間の外。
//     void run();                         // 接尾辞配列を作る。ここだけ測る。
//     const vector<int> &answer() const;  // 結果を取り出す。整形は外。
//   };
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-zalgorithm と同じ形)。文字を整数の列に
// 直す処理は実装ごとに違い、組み立ての一部なので run の中に置く。設計は algo-notes の notes/sort-problems.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-suffix-array.hpp"
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
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
