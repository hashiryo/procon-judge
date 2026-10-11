// https://onlinejudge.u-aizu.ac.jp/problems/1549
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);   // 列から作る。計測区間の中。
//     int closest(int l, int r, int d);        // a[l, r) の |a_i - d| の最小。l < r
//   };
//
// 構築とクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。入力の r は閉区間の右端なので、
// ハーネスが 1 を足して渡す。N, Q は 10^5 以下。wavelet matrix では、d 未満の最大と d 以上の最小 (前後の値) を引く。
// 設計は algo-notes の notes/wavelet-matrix.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  vector<int> ans(q);

  auto t0 = chrono::steady_clock::now();
  Solver s(a);
  for (int i = 0; i < q; ++i) ans[i] = s.closest(qs[i][0], qs[i][1] + 1, qs[i][2]);
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
