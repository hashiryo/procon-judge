// 引き算の無い半環の部分集合畳み込みと exp (自作)。入力は gen.py の説明のとおり。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int t, int n, const vector<i64> &f, const vector<i64> &g);  // 構築。計測区間の外。
//     void run();                                                        // 畳み込みか exp。ここだけ測る。
//     vector<i64> answer() const;                                        // 2^n 個
//   };
//
// t は演算と半環と値の型を決める。exp は h[∅] = 1 (足し算の 0)、h[S] = S を塊に分けて塊ごとの f を掛けたものの和 (f[∅] は読まない)。
//   0: (min, +) の部分集合畳み込み h[S] = min_{T ⊆ S} f[T] + g[S \ T]。値は int で、-1 は「無い」(INF)。
//   1: (max, +) の部分集合畳み込み。値は long long で、-10^18 は「無い」(-INF)。
//   2: (min, +) の exp (塊の値の和の最小)。値は int で、-1 は「無い」。
//   3: (max, +) の exp。値は long long で、-10^18 は「無い」。
//   4: (max, min) の部分集合畳み込み h[S] = max_{T ⊆ S} min(f[T], g[S \ T])。値は int。
// g は t が 0、1、4 のときだけ読む (ほかは空)。答えも同じ決まりで出す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t, n;
  must_scan(scanf("%d %d", &t, &n), 2);
  vector<i64> f = read_ints(1 << n), g;
  if (t == 0 || t == 1 || t == 4) g = read_ints(1 << n);

  Solver sol(t, n, f, g);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
