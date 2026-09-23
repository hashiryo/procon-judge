// https://onlinejudge.u-aizu.ac.jp/problems/2803
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int k, int n, const vector<array<i64, 3>> &edges);  // {a, b, 容量}
//     void run();                                                // ここだけ測る。
//     i64 answer() const;
//     bool unbounded() const;   // 上限まで流れたら true ("overfuro" を出す)
//   };
//
// 辺は双方向。源から 1..k へ上限つきの辺を張って最大流を求めてから、最小カット
// に乗る辺を 1 本だけ上限まで広げたときの増分を見る。グラフを組むところも
// run() の中に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-dinic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int k, n, m;
  must_scan(scanf("%d %d %d", &k, &n, &m), 3);
  vector<array<i64, 3>> edges(m);
  for (auto &e : edges) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);

  Solver sol(k, n, edges);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  if (sol.unbounded()) puts("overfuro");
  else printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
