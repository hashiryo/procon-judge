// https://onlinejudge.u-aizu.ac.jp/challenges/sources/VPC/UTPC/2270
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &x, const vector<array<int, 2>> &edges,
//            const vector<array<int, 3>> &qs);   // qs は {v, w, l}
//     void run();                                // ここだけ測る。
//     const vector<i64> &answer() const;
//   };
//
// 頂点は 1 から n までで、0 を仮の根として使う (頂点 1 の親)。辺は入力の
// まま 1-indexed で渡す。
//
// 根からの経路ごとに永続な数え上げを持って、4 本の版を同時に降りて二分探索
// する。比べたいのは永続な木の持ち方。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-dynamic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> x(n + 1, 0);
  for (int i = 1; i <= n; ++i) must_scan(scanf("%lld", &x[i]), 1);
  vector<array<int, 2>> edges(n - 1);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  Solver sol(n, x, edges, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
