// https://cses.fi/problemset/task/2132/
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &x);  // 構築。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 要素を 1 ずつ動かして広義単調増加にするときの操作回数の最小値。yuki-1077 と同じ
// 形で、値の範囲が 1e9 まで。区分線形凸関数 (slope trick) を主形式で持つか共役形式
// で持つかを比べる。テストデータは CSES から手で取り込む (source = manual)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-primal.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> x = read_ints(n);

  Solver sol(x);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
