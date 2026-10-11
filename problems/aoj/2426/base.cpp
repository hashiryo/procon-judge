// https://onlinejudge.u-aizu.ac.jp/problems/2426
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<int, 2>> &points);   // 点 {x, y} から作る。計測区間の中。
//     int count(int l, int d, int r, int u);                  // [l, r) x [d, u) に入る点の数
//   };
//
// 構築とクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。入力の長方形は閉区間なので、
// ハーネスが右端と上端に 1 を足して渡す。n は 5000 以下、m は 5 * 10^5 以下、座標は |v| <= 10^9 で負もあり、
// 同じ座標の点もある (全部の点が同じ座標のケースもある)。設計は algo-notes の notes/wavelet-matrix.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/wm4.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> points(n);
  for (auto &p : points) must_scan(scanf("%d %d", &p[0], &p[1]), 2);
  vector<array<int, 4>> qs(m);
  for (auto &e : qs) must_scan(scanf("%d %d %d %d", &e[0], &e[1], &e[2], &e[3]), 4);

  vector<int> ans(m);

  auto t0 = chrono::steady_clock::now();
  Solver s(points);
  for (int i = 0; i < m; ++i) ans[i] = s.count(qs[i][0], qs[i][1], qs[i][2] + 1, qs[i][3] + 1);
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
