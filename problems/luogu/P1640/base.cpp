// 洛谷 P1640 [SCOI2010] 连续攻击游戏。装備が N 個 (N ≤ 10^6) あり、どれも属性値を 2 つ (1 以上 10^4 以下、同じでもよい)
// 持つ。装備は 1 回だけ、どちらか一方の属性値で使える。属性値 1, 2, 3, ... の順に攻撃するとき、続けて攻撃できる回数の
// 最大を答える。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &items);  // 装備ごとの 2 つの属性値
//     int run();                                          // 続けて攻撃できる回数の最大
//   };
// 入力を読んだあと、構築と run を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<array<int, 2>> items(n);
  for (auto &e : items) must_scan(scanf("%d %d", &e[0], &e[1]), 2);
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, items);
  const int ans = sol.run();
  auto t1 = chrono::steady_clock::now();
  printf("%d\n", ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
