// https://judge.yosupo.jp/problem/prefix_substring_lcs
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const string &s, const string &t, const vector<array<int, 3>> &qs);  // 構築。文字列とクエリの列を写すだけで、計測区間の外。
//     void run();                                                                // 全部のクエリの答えを求める。ここだけ測る。
//     const vector<int> &answer() const;                                         // クエリ (a, b, c) ごとに、s[0, a) と t[b, c) の
//   };                                                                           // 最長共通部分列の長さ。整形は外。
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-zalgorithm と同じ形)。クエリは全部を先に渡すので、
// 順を並べ替えて処理してよい。設計は algo-notes の notes/prefix-substring-lcs.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/psl_fenwick.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  const int q = int(read_ints(1)[0]);
  const string s = read_token(), t = read_token();
  vector<array<int, 3>> qs(q);
  {
    const vector<i64> v = read_ints(3 * q);
    for (int i = 0; i < q; ++i) qs[i] = {int(v[3 * i]), int(v[3 * i + 1]), int(v[3 * i + 2])};
  }

  Solver sol(s, t, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> &a = sol.answer();
  string out;
  out.reserve(a.size() * 5);
  for (const int x : a) out += to_string(x), out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
