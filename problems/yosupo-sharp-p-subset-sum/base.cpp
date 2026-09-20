// https://judge.yosupo.jp/problem/sharp_p_subset_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int t, const vector<i64> &s);  // s は N 個。どれも 1 以上 T 以下
//     void run();                           // 数え上げ。ここだけ測る。
//     vector<i64> answer() const;           // t = 1..T の T 個
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。どちらの実装も同じ値の個数を
// 数えるところから始まるので、その前処理も run() の中に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-fps-exp.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, t;
  must_scan(scanf("%d %d", &n, &t), 2);
  vector<i64> s = read_ints(n);

  Solver sol(t, s);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
