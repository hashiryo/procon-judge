// https://onlinejudge.u-aizu.ac.jp/challenges/sources/JAG/Summer/2450
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &w, const vector<array<int, 2>> &edges);
//     void assign(int u, int v, i64 c);  // u から v への道を全部 c にする
//     i64 max_sum(int u, int v);         // その道の上の連続部分列の和の最大
//   };
//
// 頂点は 0-indexed で渡す。空の部分列は選べないので、答えは少なくとも 1 頂点
// ぶんになる。
//
// 構築を計測区間に入れてある。HLD は木を列に潰して向きの違う 2 本を持ち、
// Link-Cut 木は N 回の link を積むので、構築そのものが比較の対象になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld-segtree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> w = read_ints(n);
  vector<array<int, 2>> edges(n - 1);
  for (auto &e : edges) {
    must_scan(scanf("%d %d", &e[0], &e[1]), 2);
    --e[0], --e[1];
  }
  vector<array<i64, 4>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld %lld", &e[0], &e[1], &e[2], &e[3]), 4);
    --e[1], --e[2];
    if (e[0] == 2) ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, w, edges);
  for (auto &e : qs) {
    int u = (int)e[1], v = (int)e[2];
    if (e[0] == 1) s.assign(u, v, e[3]);
    else ans.push_back(s.max_sum(u, v));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
