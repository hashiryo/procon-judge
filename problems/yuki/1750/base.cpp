// https://yukicoder.me/problems/no/1750
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, i64 days, const vector<array<int, 2>> &edges);  // 計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;  // T 日目の都市 0 の感染者数 mod 998244353
//   };
//
// 隣接行列の T 乗の (0, 0) 成分。行列をそのまま繰り返し 2 乗するか、ベクトル列の
// 最小多項式を求めて次数を落とすか (密行列と疎な写像の 2 通り) を比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-matrix-pow.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  i64 days;
  must_scan(scanf("%d %d %lld", &n, &m, &days), 3);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver sol(n, days, edges);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
