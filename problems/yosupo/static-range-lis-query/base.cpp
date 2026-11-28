// https://judge.yosupo.jp/problem/static_range_lis_query
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。p は 0..n-1 の順列、queries は Q 個のクエリ。
//     Solver(const vector<int> &p, const vector<array<int, 2>> &queries);
//     void run();                          // ここだけ測る。
//     const vector<int> &answers() const;  // クエリの答えを聞かれた順に
//   };
//
// queries[i] は i 番目のクエリで、e[0] = l、e[1] = r (0 <= l <= r <= n)。
// 答えるのは (P_l, ..., P_{r-1}) の最長増加部分列の長さ。
// n, q は 10^5 以下、P は 0..n-1 の順列。値はすべて int で渡す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> p(n);
  for (auto &x : p) must_scan(scanf("%d", &x), 1);
  vector<array<int, 2>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver sol(p, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
