// LOJ 2042「CQOI2016」不同的最小割。N 頂点 M 辺の無向連通グラフで、全部の頂点の組の最小カットの値が何種類あるかを
// 答える (N ≤ 850、M ≤ 8500、容量 ≤ 10^5)。Gomory-Hu 木の辺の重みの種類数が答え (どの組の値も木の道の上の最小の
// 辺の重みで、木の辺の重みはその両端の組の値なので)。
//
// 提出は self-flow-gomory-hu と同じものを実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &edges);  // 無向辺 {u, v, 容量}。頂点は 0 から n - 1
//     void run();                                        // Gomory-Hu 木を作る
//     const vector<array<i64, 3>> &tree() const;         // 木の辺 {u, v, 重み} を n - 1 本
//   };
// 入力を読んだあと、構築と run を測る。種類を数えるのは計測区間の外。
#include "pj.hpp"
#include <algorithm>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
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
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, edges);
  sol.run();
  auto t1 = chrono::steady_clock::now();
  vector<i64> w;
  for (auto &e : sol.tree()) w.push_back(e[2]);
  sort(w.begin(), w.end());
  printf("%d\n", (int)(unique(w.begin(), w.end()) - w.begin()));
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
