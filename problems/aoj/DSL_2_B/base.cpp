// https://onlinejudge.u-aizu.ac.jp/courses/library/3/DSL/2/DSL_2_B
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);       // a[0..n) を 0 で初期化。計測区間の外。
//     void add(int p, i64 x);       // a[p] += x
//     i64 sum(int l, int r);        // a[l] + ... + a[r-1]
//   };
//
// AOJ の入出力は 1-indexed で区間が閉じているが、提出には 0-indexed の
// 半開区間で渡す。判定サイトごとの癖をハーネスで吸収して、提出の側は
// point_add_range_sum と同じ形にする。
//
// 計測区間にはクエリの処理だけを残す。入力の解析、Solver の構築、答えの整形は
// すべて外に出してある。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);

  Solver s(n);
  vector<i64> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.add((int)e[1] - 1, e[2]);
    else ans.push_back(s.sum((int)e[1] - 1, (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
