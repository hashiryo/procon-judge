// https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。queries[i] は i 番目のクエリで、種別ごとに次を詰める (使わない項は 0)。
//     //   0: {0, w, x, 0} -> a_w を x にする
//     //   1: {1, e, y, z} -> 辺 e の 1 次関数 (b_e, c_e) を (y, z) にする
//     Solver(int n, const vector<u32> &a, const vector<array<int, 2>> &edges,
//            const vector<u32> &b, const vector<u32> &c,
//            const vector<array<u32, 4>> &queries);
//     void run();                          // ここだけ測る。
//     const vector<u32> &answers() const;  // クエリごとの答えを Q 個 (mod 998244353)
//   };
//
// 木は頂点 0 を根とする。辺 i は edges[i] = {u_i, v_i} で、1 次関数 f_i(x) = b_i x + c_i を持つ。
// 頂点 v の P(v) は、0 から v への道の辺の関数を v の側から順に a_v に当てた値。
// 各クエリの後で sum_v P(v) mod 998244353 を返す。値はすべて 998244353 未満の整数でやりとりする。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<u32> a(n);
  for (auto &x : a) must_scan(scanf("%u", &x), 1);
  vector<array<int, 2>> edges(n - 1);
  vector<u32> b(n - 1), c(n - 1);
  for (int i = 0; i + 1 < n; ++i) {
    must_scan(scanf("%d %d %u %u", &edges[i][0], &edges[i][1], &b[i], &c[i]), 4);
  }

  // 種別で項目数が違う (0 は 3 項、1 は 4 項) ので、種別を読んでから残りを読む。
  vector<array<u32, 4>> qs(q);
  for (auto &e : qs) {
    e[3] = 0;
    must_scan(scanf("%u %u %u", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 1) must_scan(scanf("%u", &e[3]), 1);
  }

  Solver sol(n, a, edges, b, c, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
