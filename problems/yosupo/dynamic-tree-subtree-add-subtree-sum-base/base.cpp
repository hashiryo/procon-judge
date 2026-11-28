// https://judge.yosupo.jp/problem/dynamic_tree_subtree_add_subtree_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges);
//     void cut(int u, int v);                        // 辺 (u, v) を切る
//     void link(int u, int v);                       // 辺 (u, v) を張る
//     void subtree_add(int v, int p, i64 x);         // p を親とした v の部分木に x を足す
//     i64 subtree_sum(int v, int p);                 // p を親とした v の部分木の総和
//   };
//
// 構築を計測区間に入れてある。木を持つ構造どうしを、入出力を除いて比べるための問題。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo-ett.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);
  vector<pair<int, int>> edges(n - 1);
  for (auto &e : edges) must_scan(scanf("%d %d", &e.first, &e.second), 2);

  // 0 u v w x と 1 v p x と 2 v p で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    int k = e[0] == 0 ? 5 : (e[0] == 1 ? 4 : 3);
    for (int i = 1; i < k; ++i) must_scan(scanf("%lld", &e[i]), 1);
    if (e[0] == 2) ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, a, edges);
  for (auto &e : qs) {
    if (e[0] == 0) s.cut((int)e[1], (int)e[2]), s.link((int)e[3], (int)e[4]);
    else if (e[0] == 1) s.subtree_add((int)e[1], (int)e[2], e[3]);
    else ans.push_back(s.subtree_sum((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
