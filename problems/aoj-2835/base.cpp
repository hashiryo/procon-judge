// https://onlinejudge.u-aizu.ac.jp/challenges/sources/VPC/RUPC/2835
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &edges);  // {s, t, 容量}
//     void run();                                         // ここだけ測る。
//     i64 answer() const;
//   };
//
// 辺は双方向。最大流を求めてから、最小カットに乗る容量 1 の辺を 1 本ずつ
// 落として流量が減るかを見る。グラフを組むところも run() の中に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-dinic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<i64, 3>> edges(m);
  for (auto &e : edges) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);

  Solver sol(n, edges);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
