// https://yukicoder.me/problems/no/1216
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &edges,   // {a, b, c}: 街 a と b を結ぶ川、流れる時間 c
//            const vector<array<i64, 4>> &qs);            // {type, v, t, l}
//     void run();                                          // ここだけ測る。
//     const vector<i64> &answer() const;                   // 回答クエリごとの灯籠の数
//   };
//
// 街 1 (添字 0) を根とする木。灯籠は根へ向かって流れて l だけ経つと消える。木を
// 列に潰して、追加を 2 次元の点への加算に、回答を長方形の総和に直すところまでは
// どの実装も同じなので common.hpp に置く。クエリを全部読んでから解く (点の集合を
// 先に固める) 形になるので、1 回の計算の形で渡す。比べたいのは 2 次元の点集合に
// 対する 1 点更新と長方形の総和を取る構造で、kd 木と 2 次元セグメント木。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-2d.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 3>> edges(n - 1);
  for (auto &e : edges) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    --e[0], --e[1];
  }
  vector<array<i64, 4>> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld %lld", &e[0], &e[1], &e[2], &e[3]), 4);
    --e[1];
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
