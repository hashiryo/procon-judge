// https://onlinejudge.u-aizu.ac.jp/problems/3142
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges, const vector<i64> &d);
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 頂点は 0-indexed で渡す。d[i] は c[i] - b[i]。木の接続行列の連立方程式を
// 解いて各辺の値を出し、重み付き Union-Find でポテンシャルに積み直す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-union-find.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<array<int, 2>> edges(n - 1);
  for (auto &e : edges) {
    must_scan(scanf("%d %d", &e[0], &e[1]), 2);
    --e[0], --e[1];
  }
  vector<i64> d = read_ints(n);
  for (int i = 0; i < n; ++i) {
    i64 b;
    must_scan(scanf("%lld", &b), 1);
    d[i] -= b;
  }

  Solver sol(n, edges, d);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
