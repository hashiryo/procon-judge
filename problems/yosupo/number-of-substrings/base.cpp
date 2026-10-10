// https://judge.yosupo.jp/problem/number_of_substrings
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);  // 構築。文字列を写すだけで、計測区間の外。
//     void run();                        // 異なる部分文字列の数を求める。ここだけ測る。
//     long long answer() const;          // 結果を取り出す。
//   };
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-suffixarray と同じ形)。答えは長さ n の文字列で
// n (n + 1) / 2 から隣り合う接尾辞の LCP の和を引いたもので、接尾辞配列を作る処理も run の中に置く。接尾辞配列と LCP 配列を一緒に
// 求める実装とも比べられるようにするため。設計は algo-notes の notes/lcp-array.md。
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

  printf("%lld\n", sol.answer());

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
