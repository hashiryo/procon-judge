// https://yukicoder.me/problems/no/1077
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &y);  // 構築。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 高さを整数だけ動かして広義単調増加にするときの、移動距離の総和の最小値。
// 区分線形凸関数 (slope trick) を主形式で持つか共役形式で持つかを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-primal.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> y = read_ints(n);

  Solver sol(y);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
