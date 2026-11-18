// 洛谷 P5632【模板】Stoer-Wagner。n 頂点 (n ≤ 600) の連結な無向グラフ (辺の重みは正で、和は 10^9 以下) の全域最小カットの
// 値を答える。
//
// 提出は次を実装する (_shared/gmincut/solver.hpp)。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &e);  // 無向辺 {u, v, 重み}。頂点は 0 始まり
//     i64 run();                                      // 全域最小カットの値
//   };
// 入力を読んだあと、構築と run を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<i64, 3>> e(m);
  for (auto &[u, v, w] : e) must_scan(scanf("%lld %lld %lld", &u, &v, &w), 3), --u, --v;
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, e);
  const i64 ans = sol.run();
  auto t1 = chrono::steady_clock::now();
  printf("%lld\n", ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
