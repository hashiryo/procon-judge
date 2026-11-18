// https://codeforces.com/problemset/problem/1139/E
//
// 学生 n 人と部 m 個 (1 ≤ m ≤ n ≤ 5000)。学生 i は力 p_i (0 ≤ p_i < 5000) を持ち、部 c_i に属す。d 日のあいだ、毎朝 1 人が
// 部を去る。毎日の午後、部ごとに高々 1 人を選んで作るチームの力の mex の最大を答える。
//
// 提出は次を実装する。
//   struct Solver {
//     // c は 0 始まりの部の番号、k は 0 始まりの、日ごとに去る学生の番号
//     Solver(int n, int m, const vector<int> &p, const vector<int> &c, const vector<int> &k);
//     vector<int> run();  // 日ごとの答え
//   };
// 入力を読んだあと、構築と run を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<int> p(n), c(n);
  for (auto &x : p) must_scan(scanf("%d", &x), 1);
  for (auto &x : c) must_scan(scanf("%d", &x), 1), --x;
  int d;
  must_scan(scanf("%d", &d), 1);
  vector<int> k(d);
  for (auto &x : k) must_scan(scanf("%d", &x), 1), --x;
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, m, p, c, k);
  const vector<int> ans = sol.run();
  auto t1 = chrono::steady_clock::now();
  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
