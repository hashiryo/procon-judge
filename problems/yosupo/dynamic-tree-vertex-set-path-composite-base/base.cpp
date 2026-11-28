// https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<pair<u32, u32>> &a, const vector<pair<int, int>> &edges);
//     void cut(int u, int v);
//     void link(int u, int v);
//     void set(int v, u32 c, u32 d);       // 頂点 v の一次関数を (c, d) にする
//     u32 prod(int u, int v, u32 x);       // u から v への順に合成して x に当てた値
//   };
//
// 構築を計測区間に入れてある。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo-lct.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<pair<u32, u32>> a(n);
  for (auto &e : a) must_scan(scanf("%u %u", &e.first, &e.second), 2);
  vector<pair<int, int>> edges(n - 1);
  for (auto &e : edges) must_scan(scanf("%d %d", &e.first, &e.second), 2);

  vector<array<u32, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%u", &e[0]), 1);
    int k = e[0] == 0 ? 5 : 4;
    for (int i = 1; i < k; ++i) must_scan(scanf("%u", &e[i]), 1);
    if (e[0] == 2) ++gets;
  }

  vector<u32> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, a, edges);
  for (auto &e : qs) {
    if (e[0] == 0) s.cut((int)e[1], (int)e[2]), s.link((int)e[3], (int)e[4]);
    else if (e[0] == 1) s.set((int)e[1], e[2], e[3]);
    else ans.push_back(s.prod((int)e[1], (int)e[2], e[3]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
