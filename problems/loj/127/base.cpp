// https://loj.ac/p/127
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int s, int t, const vector<array<int, 3>> &edges);  // {u, v, 容量}
//     void run();         // s から t への最大流を求める
//     i64 answer() const;  // 最大流の値
//   };
//
// 頂点は 0-indexed にして渡す。容量は 1 以上 2^31 - 1 以下で、辺は有向。多重辺と自己ループもありうる。
// 構築と run を測る。ライブラリの関数として呼ぶときはグラフを組む時間も込みになるので、構築に寄せた実装が
// 得をしないようにする。edges は計測区間のあとまで生きているので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-push-relabel.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, s, t;
  must_scan(scanf("%d %d %d %d", &n, &m, &s, &t), 4);
  vector<array<int, 3>> edges(m);
  for (auto &e : edges) {
    must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);
    --e[0], --e[1];
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, s - 1, t - 1, edges);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
