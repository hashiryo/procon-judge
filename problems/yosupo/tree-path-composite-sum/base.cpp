// https://judge.yosupo.jp/problem/tree_path_composite_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<u32> &a, const vector<array<int, 2>> &edges, const vector<u32> &b,
//            const vector<u32> &c);         // 構築。計測区間の外。
//     void run();                          // 頂点ごとの答えを作る。ここだけ測る。
//     const vector<u32> &answer() const;   // 頂点 r の答えを r の順に N 個 (mod 998244353)。整形は計測区間の外。
//   };
//
// 辺 i は u_i と v_i を結び、1 次関数 f_i(x) = b_i x + c_i を持つ。頂点 r の答えは、すべての頂点 v について、v から r への
// 道の辺の関数を v の側から順に a_v に当てた値の和。全方位木 DP (Rerooting) の書き比べに使う。
// 木を作るところから run() に入れる。値の型への写し (構築) と答えの写しは計測の外にして、剰余の表し方の違いを外す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<u32> a(n);
  for (auto &x : a) must_scan(scanf("%u", &x), 1);
  vector<array<int, 2>> edges(n - 1);
  vector<u32> b(n - 1), c(n - 1);
  for (int i = 0; i + 1 < n; ++i) must_scan(scanf("%d %d %u %u", &edges[i][0], &edges[i][1], &b[i], &c[i]), 4);

  Solver sol(n, a, edges, b, c);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
