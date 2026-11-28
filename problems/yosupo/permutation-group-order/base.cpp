// https://judge.yosupo.jp/problem/permutation_group_order
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<vector<int>> &generators);  // 生成元を写すだけ (計測区間の外)
//     void run();                                            // 位数を求める。ここだけ測る
//     unsigned answer() const;                               // |G| mod 998244353
//   };
//
// 置換は p[i] を i の行き先とする。合成は (p q)[i] = p[q[i]] (問題文の r_i = p_{q_i})。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/perm_closure.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, k;
  must_scan(scanf("%d %d", &n, &k), 2);
  vector<vector<int>> gen(k, vector<int>(n));
  for (auto &p : gen)
    for (auto &x : p) must_scan(scanf("%d", &x), 1);

  Solver sol(n, gen);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%u\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
