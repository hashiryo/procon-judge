// https://judge.yosupo.jp/problem/inv_of_polynomials
//
// f x + g y = gcd(f, g) となる x を、gcd の定数倍で正規化して出す。
// gcd の次数が 0 でなければ -1、x が 0 なら 0、そうでなければ x の項数と係数を出す。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64>& f, const vector<i64>& g);  // 構築。計測区間の外。
//     void run();                                         // 拡張 gcd。ここだけ測る。
//     bool impossible() const;                            // gcd の次数が 0 でない (x を出さない)
//     vector<i64> answer() const;                         // x を gcd の定数で割ったもの。0 は空
//   };
//
// 剰余は 998244353、入出力は素の整数で渡す (内部表現を提出が選べるようにするため)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-polynomial.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 int n, m;
 must_scan(scanf("%d %d", &n, &m), 2);
 vector<i64> f = read_ints(n), g = read_ints(m);

 Solver sol(f, g);

 auto t0 = chrono::steady_clock::now();
 sol.run();
 auto t1 = chrono::steady_clock::now();

 if (sol.impossible()) {
  printf("-1\n");
 } else {
  const vector<i64> x = sol.answer();
  if (x.empty()) {
   printf("0\n");
  } else {
   printf("%d\n", (int)x.size());
   print_all(x, ' ');
  }
 }

 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
