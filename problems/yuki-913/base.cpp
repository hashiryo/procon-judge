// https://yukicoder.me/problems/no/913
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
//     void run();                              // ここだけ測る。
//     const vector<i64> &answer() const;       // 木ごとの悲しさの最小値
//   };
//
// 木 i を含む区間 [l, r] を燃やしたときの悲しさ (r - l + 1)^2 + (A の和) の最小値
// を、すべての i について求める。区間の中点で分割統治して、中点をまたぐ区間の
// 最小を「行ごとの最小」の問い合わせに落とすところまではどの実装も同じなので
// common.hpp に置き、その問い合わせを Li Chao 木で取るか monotone minima で取る
// かを比べる。累積和も run() の中に置く。どの実装も同じところから始まる。
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
