// https://judge.yosupo.jp/problem/longest_common_substring
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const string &s, const string &t);  // 構築。文字列を写すだけで、計測区間の外。
//     void run();                                // 最も長い共通部分文字列を求める。ここだけ測る。
//     array<int, 4> answer() const;              // s[a, b) = t[c, d) となる {a, b, c, d}。
//   };
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-number-of-substrings と同じ形)。s と t を区切りの
// 文字でつないだ文字列の接尾辞配列と LCP を作り、別の文字列から始まる隣り合う接尾辞の LCP の最大を取る。つなぐ処理と接尾辞配列を
// 作る処理も run の中に置く。答えが複数あるときはチェッカがどれでも受ける。設計は algo-notes の notes/lcp-array.md。
#include <array>
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  string s = read_token();
  string t = read_token();

  Solver sol(s, t);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const array<int, 4> r = sol.answer();
  printf("%d %d %d %d\n", r[0], r[1], r[2], r[3]);

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
