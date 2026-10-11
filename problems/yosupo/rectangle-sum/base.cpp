// https://judge.yosupo.jp/problem/rectangle_sum
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<int, 3>> &points);   // 点 {x, y, w} から作る。計測区間の中。
//     i64 rect_sum(int l, int d, int r, int u);               // [l, r) x [d, u) に入る点の重みの和
//   };
//
// 構築とクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。先のクエリを読んでまとめて
// 処理する実装 (x で掃いて BIT に足すなど) はここでは比べない。N, Q は 2 * 10^5 以下、座標と重みは 0 以上 10^9 以下
// で、同じ座標の点もありうる。設計は algo-notes の notes/wavelet-matrix.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/wm_b32.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<int, 3>> points(n);
  for (auto &p : points) must_scan(scanf("%d %d %d", &p[0], &p[1], &p[2]), 3);
  vector<array<int, 4>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d %d", &e[0], &e[1], &e[2], &e[3]), 4);

  vector<i64> ans(q);

  auto t0 = chrono::steady_clock::now();
  Solver s(points);
  for (int i = 0; i < q; ++i) ans[i] = s.rect_sum(qs[i][0], qs[i][1], qs[i][2], qs[i][3]);
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
