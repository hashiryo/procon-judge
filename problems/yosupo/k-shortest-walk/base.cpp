// https://judge.yosupo.jp/problem/k_shortest_walk
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int s, int t, int k, vector<int> a, vector<int> b, vector<long long> c);
//     void run();                        // ここだけ測る。
//     const vector<long long> &answer() const;  // 短い順の k 本の長さ。無ければ -1。
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, s, t, k;
  must_scan(scanf("%d %d %d %d %d", &n, &m, &s, &t, &k), 5);
  vector<int> a(m), b(m);
  vector<long long> c(m);
  for (int i = 0; i < m; ++i) must_scan(scanf("%d %d %lld", &a[i], &b[i], &c[i]), 3);

  Solver sol(n, s, t, k, a, b, c);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
