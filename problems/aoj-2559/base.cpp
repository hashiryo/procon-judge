// https://onlinejudge.u-aizu.ac.jp/problems/2559
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &edges);  // {a, b, w} 0-indexed
//     void run();                         // ここだけ測る。
//     const vector<i64> &answer() const;  // 辺ごとに 1 つ
//   };
//
// 辺 i を必ず使うときの最小全域木の重みを、辺ごとに出す。作れないときは -1。
//
// 最小全域木を作るところはどちらの実装でも同じなので、共通の関数にしてある。
// そのぶんも計測区間に入るが、両方に等しく乗る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld-segtree.hpp"
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

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
