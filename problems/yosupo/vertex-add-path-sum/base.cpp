// https://judge.yosupo.jp/problem/vertex_add_path_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges);
//     void add(int p, i64 x);        // a[p] += x
//     i64 path_sum(int u, int v);    // u から v への道の上の頂点の総和 (端点を含む)
//   };
//
// 辺は無向で、根は決まっていない。提出が自分で決める。
//
// 構築を計測区間に入れてある。HLD は木を列に潰してから BIT を載せ、Link-Cut 木は
// N 回の link を積むので、構築そのものが比較の対象になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld-bit.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);
  vector<array<int, 2>> edges(n - 1);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  // 0 p x も 1 u v も 3 項目なので、まとめて読む。
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0]) ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, a, edges);
  for (auto &e : qs) {
    if (e[0] == 0) s.add((int)e[1], e[2]);
    else ans.push_back(s.path_sum((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
