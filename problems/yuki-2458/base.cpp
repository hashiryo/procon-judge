// https://yukicoder.me/problems/no/2458
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &q);  // 電荷。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 残すボールを左から順に決める DP で、遷移 max_j dp[j] + Q_j Q_i が「傾き Q_j
// 切片 dp[j] の直線を x = Q_i で評価した最大値」になる。DP は common.hpp に置き、
// 直線の集合を平衡二分探索木の凸包 (ConvexHullTrick) で持つか Li Chao 木で持つか
// を比べる。直線を入れながら問い合わせるので、構築ごと計測区間に入れる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-convex-hull-trick.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> q = read_ints(n);

  Solver sol(q);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
