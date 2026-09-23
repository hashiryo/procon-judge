// https://onlinejudge.u-aizu.ac.jp/courses/library/5/GRL/2/GRL_2_B
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int root, const vector<array<i64, 3>> &edges);  // {s, t, w}
//     void run();                                                   // ここだけ測る。
//     i64 answer() const;                                           // 最小の総重み
//   };
//
// グラフを組むところも run() の中に置く。専用の実装とマトロイド交叉では
// 持ち方が違う。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-minimum-spanning-arborescence.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, root;
  must_scan(scanf("%d %d %d", &n, &m, &root), 3);
  vector<array<i64, 3>> edges(m);
  for (auto &e : edges) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);

  Solver sol(n, root, edges);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
