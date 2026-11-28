// https://judge.yosupo.jp/problem/jump_on_tree
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<int> a, vector<int> b, vector<int> s, vector<int> t, vector<int> i);
//     void run();                       // ここだけ測る。
//     const vector<int> &answer() const;  // クエリごとに、s-t の道上の i 番目の頂点。-1 は無い。
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> a(n - 1), b(n - 1);
  for (int i = 0; i < n - 1; ++i) must_scan(scanf("%d %d", &a[i], &b[i]), 2);
  vector<int> s(q), t(q), k(q);
  for (int i = 0; i < q; ++i) must_scan(scanf("%d %d %d", &s[i], &t[i], &k[i]), 3);

  Solver sol(n, a, b, s, t, k);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
