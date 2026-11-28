// 永続セグメント木 (区間 max xor) のハーネス。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);
//     int max_xor(int l, int r, int x);  // a[l, r) の値のうち、x との xor が最大になるものの xor
//   };
//
// 構築とクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。
// N, Q は 2 * 10^5 以下、a_i と x は 0 以上 10^9 以下 (30 bit)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  vector<int> ans(q);

  auto t0 = chrono::steady_clock::now();
  Solver s(a);
  for (int i = 0; i < q; ++i) ans[i] = s.max_xor(qs[i][0], qs[i][1], qs[i][2]);
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
