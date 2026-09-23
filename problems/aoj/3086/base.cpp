// https://onlinejudge.u-aizu.ac.jp/problems/3086
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int l, const vector<i64> &a);  // 区間の最小の長さ l
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 長さ l 以上の区間に切り分けて、各区間の最大値の和を最大にする。どちらの
// 実装もコスト関数に区間最大を使うので、セグメント木を組むところも run() の
// 中に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-larsch.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, l;
  must_scan(scanf("%d %d", &n, &l), 2);
  vector<i64> a = read_ints(n);

  Solver sol(l, a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
