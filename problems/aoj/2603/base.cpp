// https://onlinejudge.u-aizu.ac.jp/problems/2603
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int m, const vector<i64> &a);  // m 個の組に分ける
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// a を昇順に並べて m 個の連続する組に切り分け、各組の中で最大値との差の和を
// 最小にする。並べ替えと累積和も run() の中に置く。どの実装も同じところから
// 始まる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-monotone-minima.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int s, n, m;
  must_scan(scanf("%d %d %d", &s, &n, &m), 3);
  vector<i64> x = read_ints(s);
  vector<i64> a(n);
  for (int i = 0; i < n; ++i) {
    i64 t, p;
    must_scan(scanf("%lld %lld", &t, &p), 2);
    a[i] = t - x[p - 1];
  }

  Solver sol(m, a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
