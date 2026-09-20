// https://onlinejudge.u-aizu.ac.jp/problems/2159
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<i64, 2>> &ps);  // 構築。計測区間の外。
//     void run();           // ここだけ測る。
//     bool answer() const;  // 線対称なら true
//   };
//
// 座標は整数。比べたいのは座標の型 (浮動小数点数と有理数) なので、解法は
// 1 つにして提出は型を選ぶだけにしてある。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-long-double.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<array<i64, 2>> ps(n);
  for (auto &p : ps) must_scan(scanf("%lld %lld", &p[0], &p[1]), 2);

  Solver sol(ps);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  puts(sol.answer() ? "Yes" : "No");

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
