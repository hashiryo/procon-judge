// https://onlinejudge.u-aizu.ac.jp/problems/2865
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &d, const vector<i64> &g);  // d は N-1 個
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// どちらも区分線形凸関数を持ち回る同じ道具を使うが、主形式で解くか共役で
// 解くかが違う。積む操作の種類と回数がそのまま差になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-primal.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> d = read_ints(n - 1);
  vector<i64> g = read_ints(n);

  Solver sol(d, g);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
