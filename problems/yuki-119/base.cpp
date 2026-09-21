// https://yukicoder.me/problems/no/119
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<array<i64, 2>> &bc,     // {ツアーに行く満足度 B, 行かない満足度 C}
//            const vector<array<int, 2>> &de);    // 条件 (D, E)
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 国ごとに「行かない / 行くがツアーは無し / ツアーも行く」の 3 値を選ぶ最小カットに
// 帰着する。帰着は common.hpp に置き、最大流のエンジン (Dinic / Push-Relabel) を
// 比べる。N <= 40 なので小さい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-dinic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<array<i64, 2>> bc(n);
  for (auto &e : bc) must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);
  int m;
  must_scan(scanf("%d", &m), 1);
  vector<array<int, 2>> de(m);
  for (auto &e : de) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver sol(bc, de);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
