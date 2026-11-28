// https://judge.yosupo.jp/problem/counting_c4
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。辺 i は a[i] と b[i] を結ぶ。多重辺があってよい。
//     Solver(int n, vector<int> a, vector<int> b);
//     void run();                                // ここだけ測る。
//     const vector<long long> &answers() const;  // 辺 i を含む C4 の個数
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<int> a(m), b(m);
  for (int i = 0; i < m; ++i) must_scan(scanf("%d %d", &a[i], &b[i]), 2);

  Solver sol(n, a, b);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers(), ' ');

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
