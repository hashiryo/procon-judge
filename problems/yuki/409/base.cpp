// https://yukicoder.me/problems/no/409
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(i64 a, i64 b, i64 w, const vector<i64> &d);
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 運動を続けた日数の 2 乗で効く費用が Monge になるので、分割統治でも直線の
// 族でも解ける。比べたいのはその 2 通り。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-larsch.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  i64 n, a, b, w;
  must_scan(scanf("%lld %lld %lld %lld", &n, &a, &b, &w), 4);
  vector<i64> d = read_ints((int)n);

  Solver sol(a, b, w, d);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
