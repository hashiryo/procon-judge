// https://onlinejudge.u-aizu.ac.jp/courses/library/5/GRL/7/GRL_7_A
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int x, int y, const vector<array<int, 2>> &edges);
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 最大マッチングの本数
//   };
//
// 左の頂点は [0, x)、右の頂点は [0, y) で番号を分けて渡す。グラフを組む
// ところも run() の中に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-matroid-intersection.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int x, y, m;
  must_scan(scanf("%d %d %d", &x, &y, &m), 3);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver sol(x, y, edges);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
