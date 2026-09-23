// https://onlinejudge.u-aizu.ac.jp/problems/2893
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &edges);  // {u, v, w} 0-indexed
//     void run();                        // ここだけ測る。
//     array<i64, 2> answer() const;      // 選ぶ辺の端点 (0-indexed)
//   };
//
// 同じ費用の辺が複数あるときは、端点の組が辞書順で最小のものを返す。
// 答えは 1-indexed で出すので、ハーネスが足す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-incremental-bridge.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<i64, 3>> edges(m);
  for (auto &e : edges) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    --e[0], --e[1];
  }

  Solver sol(n, edges);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  auto a = sol.answer();
  printf("%lld %lld\n", a[0] + 1, a[1] + 1);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
