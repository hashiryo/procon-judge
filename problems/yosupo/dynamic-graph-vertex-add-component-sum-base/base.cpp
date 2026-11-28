// https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &a);
//     void link(int u, int v);
//     void cut(int u, int v);
//     void add(int v, i64 x);               // a[v] += x
//     i64 component_sum(int v);             // v の連結成分の総和
//   };
//
// オンラインで答える構造どうしを、入出力を除いて比べるための問題。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo-odc.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);

  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);
    if (e[0] != 3) must_scan(scanf("%lld", &e[2]), 1);
    if (e[0] == 3) ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, a);
  for (auto &e : qs) {
    if (e[0] == 0) s.link((int)e[1], (int)e[2]);
    else if (e[0] == 1) s.cut((int)e[1], (int)e[2]);
    else if (e[0] == 2) s.add((int)e[1], e[2]);
    else ans.push_back(s.component_sum((int)e[1]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
