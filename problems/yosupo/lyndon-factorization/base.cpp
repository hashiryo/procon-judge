// https://judge.yosupo.jp/problem/lyndon_factorization
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);   // 構築。文字列を写すだけで、計測区間の外。
//     void run();                         // Lyndon 分解を求める。ここだけ測る。
//     const vector<int> &answer() const;  // 切れ目の列 0 = a_0 < a_1 < ... < a_k = n。整形は外。
//   };
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-zalgorithm と同じ形)。設計は algo-notes の
// notes/string-problems.md と notes/lyndon.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/duval.hpp"
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
