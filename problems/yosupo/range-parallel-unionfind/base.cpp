// https://judge.yosupo.jp/problem/range_parallel_unionfind
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。queries[i] = {k, a, b} が i 番目のクエリ。
//     Solver(int n, vector<int> x, vector<array<int, 3>> queries);
//     void run();                          // ここだけ測る。
//     const vector<int> &answers() const;  // 各クエリを処理した後の X mod 998244353
//   };
//
// クエリ i では j = 0, 1, ..., k-1 について辺 (a+j, b+j) を足す (すべての頂点番号は N 未満)。
// x_i は 0 以上 998244352 以下。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> x(n);
  for (auto &v : x) must_scan(scanf("%d", &v), 1);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  Solver sol(n, std::move(x), std::move(qs));

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
