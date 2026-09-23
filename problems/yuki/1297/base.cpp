// https://yukicoder.me/problems/no/1297
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, i64 c, const vector<array<i64, 2>> &ab);  // {a_i, b_i}。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 島を左から区間に切って各区間を 1 人の職人に任せる DP で、遷移が直線の集合の
// 最小値の問い合わせになる。DP は common.hpp に置き、直線の集合を平衡二分探索木の
// 凸包 (ConvexHullTrick) で持つか Li Chao 木で持つかを比べる。直線を入れながら
// 問い合わせるので、構築ごと計測区間に入れる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-convex-hull-trick.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  i64 c;
  must_scan(scanf("%d %lld", &n, &c), 2);
  vector<array<i64, 2>> ab(n);
  for (auto &e : ab) must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);

  Solver sol(n, c, ab);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
