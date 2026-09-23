// https://judge.yosupo.jp/problem/unionfind_with_potential
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);          // 構築。計測区間の外。
//     int unite(int u, int v, i64 x);  // a[u] = a[v] + x。矛盾しなければ 1
//     i64 diff(int u, int v);          // a[u] - a[v]。定まらなければ -1
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。
//
// 構築は n 個の要素を並べるだけの O(N) なので、計測区間の外に置く。クエリ列が
// O(Q α(N)) で、そちらが比較の対象になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-union-find.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);

  // 0 u v x と 1 u v で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 4>> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 0) must_scan(scanf("%lld", &e[3]), 1);
  }

  Solver s(n);
  vector<i64> ans;
  ans.reserve(q);  // どちらの種別も 1 行出す

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) ans.push_back(s.unite((int)e[1], (int)e[2], e[3]));
    else ans.push_back(s.diff((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
