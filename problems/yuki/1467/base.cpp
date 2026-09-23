// https://yukicoder.me/problems/no/1467
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &a, const vector<i64> &b);  // 客の希望色、在庫の色。計測区間の外。
//     void run();                                          // ここだけ測る。
//     const vector<i64> &answer() const;                   // k = 1 .. M ごとの最小値
//   };
//
// 客 M 人に在庫 N 色 x k 台から 1 台ずつ売るときの、色の差の総和の最小値を k
// ごとに求める。数直線上の最小費用マッチングで、色ごとに客と在庫の差を運ぶ量を
// 変数にした凸関数を左から右へ積んでいく。k ごとに独立に解く。座標圧縮と個数の
// 数え上げは common.hpp に置き、区分線形凸関数 (slope trick) を主形式で持つか
// 共役形式で持つかを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-primal.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int m, n;
  must_scan(scanf("%d %d", &m, &n), 2);
  vector<i64> a = read_ints(m);
  vector<i64> b = read_ints(n);

  Solver sol(a, b);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
