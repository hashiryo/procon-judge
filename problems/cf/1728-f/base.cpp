// https://codeforces.com/problemset/problem/1728/F
//
// 漁師 n 人 (n ≤ 1000) が順に魚の大きさを言う。1 人目は本当の大きさ a を言い、2 人目からは、前の人の値より大きい自分の
// a の倍数の最小を言う。言う値の和の最小を答える (a_i ≤ 10^6)。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<long long> &a);
//     long long run();  // 言う値の和の最小
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
  const vector<long long> a = read_ints(n);
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, a);
  const long long ans = sol.run();
  auto t1 = chrono::steady_clock::now();
  printf("%lld\n", ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
