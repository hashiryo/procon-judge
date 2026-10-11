// https://judge.yosupo.jp/problem/range_kth_smallest
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);   // 列から作る。計測区間の中。
//     int kth_smallest(int l, int r, int k);   // a[l, r) の小さい方から k 番目 (0 始まり)。0 <= k < r - l
//   };
//
// 構築とクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。先のクエリを読んでまとめて
// 処理する実装はここでは比べない。N, Q は 2 * 10^5 以下、a_i は 0 以上 10^9 以下。設計は algo-notes の
// notes/wavelet-matrix.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
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
  for (int i = 0; i < q; ++i) ans[i] = s.kth_smallest(qs[i][0], qs[i][1], qs[i][2]);
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
