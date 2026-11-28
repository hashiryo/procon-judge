// https://judge.yosupo.jp/problem/static_range_mode_query
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。a は長さ N の列、queries は Q 個のクエリ。
//     Solver(const vector<int> &a, const vector<array<int, 2>> &queries);
//     void run();                                    // ここだけ測る。
//     const vector<array<int, 2>> &answers() const;  // {最頻値, その出現回数} をクエリの順に
//   };
//
// queries[i] は {l, r} で、a[l, r) の最頻値とその出現回数を出す。0 <= l < r <= N。
// 最頻値が複数あるときはどれを出してもよい。a_i は 0 以上 10^9 以下で int に収まる。
// 出力は 1 クエリ 1 行で "x freq"。Q = 0 なら何も書かない。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);
  vector<array<int, 2>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver sol(a, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const auto &ans = sol.answers();
  string out;
  out.reserve(ans.size() * 24);
  for (const auto &p : ans) {
    out += to_string(p[0]);
    out += ' ';
    out += to_string(p[1]);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
