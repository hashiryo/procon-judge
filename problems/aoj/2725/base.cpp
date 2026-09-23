// https://onlinejudge.u-aizu.ac.jp/problems/2725
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int t, const vector<array<i64, 3>> &items);  // {締切, 得点, 時刻}
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 締切ごとの DP を回すときに「時刻の差の 2 乗」ぶんの損が入るので、直線の
// 集合に対する最大値の問い合わせに落ちる。並べ替えも run() の中に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-convex-hull-trick.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, t;
  must_scan(scanf("%d %d", &n, &t), 2);
  vector<array<i64, 3>> items(n);
  for (auto &e : items) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);

  Solver sol(t, items);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
