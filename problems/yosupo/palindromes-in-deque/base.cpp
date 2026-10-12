// https://judge.yosupo.jp/problem/palindromes_in_deque
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<pair<int, char>> &qs);  // 構築。クエリの列を写すだけで、計測区間の外。
//     void run();                                          // クエリを順に処理する。ここだけ測る。
//     const vector<array<int, 3>> &answer() const;          // 各クエリのあとの (異なる回文の数, 最長の回文接頭辞の長さ,
//   };                                                     // 最長の回文接尾辞の長さ)。整形は外。
//
// クエリは (種類, 文字) で、種類 0 は先頭に、1 は末尾に文字を足し、2 は先頭を、3 は末尾を除く (2 と 3 の文字は使わない)。全部の
// クエリを先に渡すが、提出は前から順に処理し、各クエリのあとの答えを残す。設計は algo-notes の notes/palindromes-in-deque.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/dq_basic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  const int q = stoi(read_token());
  vector<pair<int, char>> qs(q);
  for (auto &[t, c] : qs) {
    t = stoi(read_token());
    c = 0;
    if (t <= 1) c = read_token()[0];
  }

  Solver sol(qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<array<int, 3>> &a = sol.answer();
  string out;
  out.reserve(a.size() * 24);
  for (const auto &[x, y, z] : a) out += to_string(x) + ' ' + to_string(y) + ' ' + to_string(z) + '\n';
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
