// https://judge.yosupo.jp/problem/static_rectangle_add_rectangle_sum
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。渡された列を写すだけで、計算はしない。
//     Solver(const vector<array<int, 5>> &adds,      // N 個の加算 {l, d, r, u, w}
//            const vector<array<int, 4>> &queries);  // Q 個の和 {l, d, r, u}
//     void run();                             // ここだけ測る。
//     const vector<int> &answers() const;     // クエリごとの和 mod 998244353
//   };
//
// 加算 {l, d, r, u, w} は [l, r) x [d, u) の各点に w を足す。和 {l, d, r, u} は
// [l, r) x [d, u) の総和。すべての加算がすべての和より先にあるので、和もまとめて
// 渡す。オフラインで x を掃く実装を比べられる。
//
// N, Q は 2 * 10^5 以下、座標は 0 以上 10^9 以下で int に収まる。w は 0 以上
// 998244353 未満、答えは mod 998244353 で int に収まる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<int, 5>> adds(n);
  for (auto &e : adds)
    must_scan(scanf("%d %d %d %d %d", &e[0], &e[1], &e[2], &e[3], &e[4]), 5);
  vector<array<int, 4>> queries(q);
  for (auto &e : queries)
    must_scan(scanf("%d %d %d %d", &e[0], &e[1], &e[2], &e[3]), 4);

  Solver sol(adds, queries);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
