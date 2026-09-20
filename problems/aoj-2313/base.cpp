// https://onlinejudge.u-aizu.ac.jp/challenges/sources/JAG/Summer/2313
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges,
//            const vector<array<int, 3>> &qs);   // qs は {種別, a, b}
//     void run();                                // ここだけ測る。
//     const vector<i64> &answer() const;         // クエリごとに 1 つ
//   };
//
// 頂点も辺も 0-indexed にして渡す。辺は双方向で容量 1。クエリは辺を 1 本
// 足すか抜くかで、そのたびに 0 から n-1 への最大流を出し直す。流量は 1 ずつ
// しか動かないので、全部を解き直さず差分だけ流す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-dinic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, q;
  must_scan(scanf("%d %d %d", &n, &m, &q), 3);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) {
    must_scan(scanf("%d %d", &e[0], &e[1]), 2);
    --e[0], --e[1];
    if (e[0] > e[1]) swap(e[0], e[1]);
  }
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);
    --e[1], --e[2];
    if (e[1] > e[2]) swap(e[1], e[2]);
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
