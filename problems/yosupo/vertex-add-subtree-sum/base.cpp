// https://judge.yosupo.jp/problem/vertex_add_subtree_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &a, const vector<int> &par);
//     void add(int u, i64 x);      // a[u] += x
//     i64 subtree_sum(int u);      // u を根とする部分木の総和
//   };
//
// 木は頂点 0 を根とする。par[i] は頂点 i の親で、par[0] は -1。
//
// 構築を計測区間に入れてある。HLD は木を列に潰してから BIT を載せ、Euler Tour
// 木は N 回の link を積むので、構築そのものが比較の対象になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld-bit.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);
  vector<int> par(n, -1);
  for (int i = 1; i < n; ++i) must_scan(scanf("%d", &par[i]), 1);

  // 0 u x と 1 u で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);
    if (e[0] == 0) must_scan(scanf("%lld", &e[2]), 1);
    else ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, a, par);
  for (auto &e : qs) {
    if (e[0] == 0) s.add((int)e[1], e[2]);
    else ans.push_back(s.subtree_sum((int)e[1]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
