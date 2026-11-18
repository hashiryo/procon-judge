// https://loj.ac/p/2338
//
// 問題が m 問、锦囊が n 種類 (n, m ≤ 1000) あり、どの锦囊も 1 回だけ使える。問題ごとに使える锦囊が 2 つ (同じでもよい) あり、
// 問題を順に、使える锦囊を 1 つずつ使って解く。続けて解ける問題の数の最大を答える。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &q);  // 問題ごとに使える 2 つの锦囊 (0 から n - 1)
//     int run();                                      // 続けて解ける問題の数の最大
//   };
// 入力を読んだあと、構築と run を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/keep2_lap.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> q(m);
  for (auto &e : q) must_scan(scanf("%d %d", &e[0], &e[1]), 2);
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, q);
  const int ans = sol.run();
  auto t1 = chrono::steady_clock::now();
  printf("%d\n", ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
