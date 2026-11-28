// https://judge.yosupo.jp/problem/static_range_sum
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。a は長さ N の列、queries は Q 個のクエリ。
//     Solver(const vector<i64> &a, const vector<array<int, 2>> &queries);
//     void run();                          // ここだけ測る。
//     const vector<i64> &answers() const;  // sum a[l, r) をクエリの順に
//   };
//
// queries[i] は {l, r}。0 <= l < r <= N。a_i は 0 以上 10^9 以下、
// N, Q は 5 * 10^5 以下なので、和は 64 bit。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a(n);
  for (auto &x : a) must_scan(scanf("%lld", &x), 1);
  vector<array<int, 2>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver sol(a, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
