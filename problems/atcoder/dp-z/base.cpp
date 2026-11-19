// https://atcoder.jp/contests/dp/tasks/dp_z
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(i64 c, const vector<i64> &h);  // 計測区間の外
//     void run();                           // ここだけ測る
//     i64 answer() const;
//   };
//
// dp_i = min_{j<i} dp_j + (h_i - h_j)^2 + C は、傾き -2 h_j 切片 dp_j + h_j^2 の直線を x = h_i で評価した最小に
// h_i^2 + C を足したもの。h は狭義に増えるので、傾きは足す順に減り、問いの x は増える。直線の集合を差し替える
// DP は common.hpp に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lines-cht-deque.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  i64 c;
  must_scan(scanf("%d %lld", &n, &c), 2);
  vector<i64> h = read_ints(n);

  Solver sol(c, h);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
