// https://judge.yosupo.jp/problem/enumerate_triangles
//
// 提出は次を実装する。
//   struct Solver {
//     // 頂点 i の値が x[i]、辺 i が {u[i], v[i]}。単純無向グラフ。構築。計測区間の外。
//     Solver(int n, vector<int> x, vector<int> u, vector<int> v);
//     void run();                // ここだけ測る。
//     long long answer() const;  // 全三角形 (a<b<c) の x_a x_b x_c の和 mod 998244353
//   };
//
// 入力は N M、x_0 ... x_{N-1}、続いて M 行の "u v" の順。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<int> x(n);
  for (int i = 0; i < n; ++i) must_scan(scanf("%d", &x[i]), 1);
  vector<int> u(m), v(m);
  for (int i = 0; i < m; ++i) must_scan(scanf("%d %d", &u[i], &v[i]), 2);

  Solver sol(n, x, u, v);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
