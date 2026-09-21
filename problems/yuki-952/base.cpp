// https://yukicoder.me/problems/no/952
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
//     void run();                              // ここだけ測る。
//     const vector<i64> &answer() const;       // k = 1 .. N 個開けるときの危険度の最小値
//   };
//
// 開いているドアの連結成分ごとに (A の和)^2 がかかる。閉めるドアを左から 1 つ
// ずつ決める DP を N 段回すと k ごとの答えが出る。DP の骨組みは common.hpp に
// 置き、1 段ぶんの遷移 min_j dp[j] + w(i, j) を Li Chao 木で取るか monotone
// minima で取るかを比べる。どちらも O(N^2 log N)。累積和も run() の中に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-monotone-minima.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> a = read_ints(n);

  Solver sol(a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
