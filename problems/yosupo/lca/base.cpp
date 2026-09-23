// https://judge.yosupo.jp/problem/lca
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<int> &par);  // 前処理。計測区間の中。
//     int lca(int u, int v);
//   };
//
// par[i] は頂点 i の親で、i >= 1 では par[i] < i が保証されている。par[0] は -1。
//
// 前処理を計測区間に入れてある。HLD は 1 回の走査で列に潰し、Link-Cut 木は
// N 回の link を積むので、前処理そのものが比較の対象になる。外に出すと、木を
// 作るのが重い実装ほど得をする。入力の解析だけを外に出す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> par(n, -1);
  for (int i = 1; i < n; ++i) must_scan(scanf("%d", &par[i]), 1);
  vector<array<int, 2>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  vector<i64> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, par);
  for (auto &e : qs) ans.push_back(s.lca(e[0], e[1]));
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
