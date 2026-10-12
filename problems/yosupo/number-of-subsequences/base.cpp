// https://judge.yosupo.jp/problem/number_of_subsequences
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<unsigned> &a);  // 構築。数列を写すだけで、計測区間の外。
//     void run();                                  // 空でない相異なる部分列の数を 998244353 で割った余りを求める。ここだけ測る。
//     unsigned answer() const;                     // 答えを返す。整形は外。
//   };
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-zalgorithm と同じ形)。値は 10^9 以下なので
// unsigned で渡す。設計は algo-notes の notes/number-of-subsequences.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/nos_sort.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  const int n = int(read_ints(1)[0]);
  const vector<i64> a64 = read_ints(n);
  const vector<unsigned> a(a64.begin(), a64.end());

  Solver sol(a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%u\n", sol.answer());

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
