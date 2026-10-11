// https://atcoder.jp/contests/abc281/tasks/abc281_e
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);    // 列から作る。計測区間の中。
//     i64 sum_smallest(int l, int r, int k);    // a[l, r) の小さい方から k 個の和。1 <= k <= r - l
//   };
//
// 構築とクエリの両方を計測区間に入れる。ハーネスが長さ M の窓を左から順に [i, i + M) として 1 つずつ渡し、その場で
// 答えさせる。窓が順にずれることを使う実装 (2 つの multiset で窓をずらす) も比べる。N は 2 * 10^5 以下、A_i は
// 0 以上 10^9 以下。設計は algo-notes の notes/wavelet-matrix.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/wm_sep.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, k;
  must_scan(scanf("%d %d %d", &n, &m, &k), 3);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);

  vector<i64> ans(n - m + 1);

  auto t0 = chrono::steady_clock::now();
  Solver s(a);
  for (int i = 0; i + m <= n; ++i) ans[i] = s.sum_smallest(i, i + m, k);
  auto t1 = chrono::steady_clock::now();

  print_all(ans, ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
