// https://yukicoder.me/problems/no/704
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &a, const vector<i64> &x, const vector<i64> &y);
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 費用 w(i, j) = pen(x[j] - a[i-1]) + pen(y[j]) が Monge なので、分割統治でも直線の族でも解ける。
// 比べたいのはその 2 通り。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-larsch.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> a = read_ints(n);
  vector<i64> x = read_ints(n);
  vector<i64> y = read_ints(n);

  Solver sol(a, x, y);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
