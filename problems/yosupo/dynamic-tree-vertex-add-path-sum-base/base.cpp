// https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_path_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges);
//     void cut(int u, int v);
//     void link(int u, int v);
//     void add(int v, i64 x);            // a[v] += x
//     i64 prod(int u, int v);            // u から v への道の総和
//   };
//
// 構築を計測区間に入れてある。木を持つ構造どうしを、入出力を除いて比べるための問題。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo-lct.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);
  vector<pair<int, int>> edges(n - 1);
  for (auto &e : edges) must_scan(scanf("%d %d", &e.first, &e.second), 2);

  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    int k = e[0] == 0 ? 5 : 3;
    for (int i = 1; i < k; ++i) must_scan(scanf("%lld", &e[i]), 1);
    if (e[0] == 2) ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, a, edges);
  for (auto &e : qs) {
    if (e[0] == 0) s.cut((int)e[1], (int)e[2]), s.link((int)e[3], (int)e[4]);
    else if (e[0] == 1) s.add((int)e[1], e[2]);
    else ans.push_back(s.prod((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
