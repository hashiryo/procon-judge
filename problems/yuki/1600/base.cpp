// https://yukicoder.me/problems/no/1600
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges,   // 辺 i の長さは 2^(i+1)
//            const vector<array<int, 3>> &qs);            // {x, y, z}: 辺 z を通らない x から y の最短路
//     void run();                                          // ここだけ測る。
//     const vector<i64> &answer() const;                   // mod 1e9+7。無ければ -1
//   };
//
// 長さが 2 の冪で全部違うので、最短路は最小全域木の上の道で、避ける辺が木の辺なら
// 木に無い辺のうち番号が最小でその切断をまたぐものを 1 本だけ足した道になる。
// 最小全域木、木の上の距離、木に無い辺を (両端の列の位置) の点にするところは
// どの実装も同じなので common.hpp に置く。比べたいのは静的な 2 次元の点集合に
// 対する長方形の最小を取る構造で、kd 木と 2 次元セグメント木。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-2d.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2), --e[0], --e[1];
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);
    --e[0], --e[1], --e[2];
  }

  Solver sol(n, edges, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
