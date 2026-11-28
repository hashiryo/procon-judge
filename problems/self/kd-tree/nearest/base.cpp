// 最近傍の自作問題。N 個の点と Q 個のクエリ (点) が与えられる。各クエリについて、
// 一番近い点までの距離の 2 乗を答える。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<array<i64, 2>> &pts,   // N 個の点
//            const vector<array<i64, 2>> &qs);   // Q 個のクエリの点
//     void run();                                 // ここだけ測る。構築もこの中で行う。
//     const vector<i64> &answers() const;         // クエリごとの距離の 2 乗
//   };
//
// 点は 0 以上 10^9 以下の座標にあり、同じ座標の点があってよい。距離の 2 乗は 2×10^18 まで。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-kd-tree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 2>> pts(n), qs(q);
  for (auto &p : pts) must_scan(scanf("%lld %lld", &p[0], &p[1]), 2);
  for (auto &p : qs) must_scan(scanf("%lld %lld", &p[0], &p[1]), 2);

  Solver sol(pts, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
