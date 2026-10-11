// https://judge.yosupo.jp/problem/enumerate_palindromes
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);   // 構築。文字列を写すだけで、計測区間の外。
//     void run();                         // 回文の長さを求める。ここだけ測る。
//     const vector<int> &answer() const;  // 長さ 2n - 1 の列。c 番目は、位置 c / 2 を中心とする最長の回文の長さ。整形は外。
//   };
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-zalgorithm と同じ形)。c が偶数なら文字 s[c / 2]
// を中心とする奇数の長さ、奇数なら s[c / 2] と s[c / 2 + 1] の間を中心とする偶数の長さ。設計は algo-notes の
// notes/z-array-palindromes.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  string s = read_token();

  Solver sol(s);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
