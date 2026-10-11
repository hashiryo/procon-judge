// https://onlinejudge.u-aizu.ac.jp/problems/2674
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &x);      // 列から作る。計測区間の中。
//     int count(int l, int r, int lo, int hi);    // x[l, r) のうち値が [lo, hi) に入るものの数
//   };
//
// 構築とクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。入力の l と r は 1 始まりの
// 閉区間なので、ハーネスが [l - 1, r) に直す。答えは区間の長さから、[min(x_l, x_r) - e, max(x_l, x_r) + e] に入る
// 値の数を引いたもの。D, Q は 10^5 以下、|x| は 10^8 以下で負もある。設計は algo-notes の notes/wavelet-matrix.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<int> x(n);
  for (auto &v : x) must_scan(scanf("%d", &v), 1);
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  vector<int> ans(q);

  auto t0 = chrono::steady_clock::now();
  Solver s(x);
  for (int i = 0; i < q; ++i) {
    const int l = qs[i][0] - 1, r = qs[i][1], e = qs[i][2];
    const int lo = min(x[l], x[r - 1]) - e, hi = max(x[l], x[r - 1]) + e + 1;
    ans[i] = (r - l) - s.count(l, r, lo, hi);
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
