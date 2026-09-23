// https://www.hackerrank.com/contests/happy-query-contest/challenges/grid-xor-query
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<array<int, 3>> &pts,    // {x, y, v}
//            const vector<array<int, 4>> &qs);    // {a, b, c, d}: a <= x <= b, c <= y <= d
//     void run();                                  // ここだけ測る。
//     const vector<i64> &answer() const;           // 質問ごとの XOR
//   };
//
// 点は動かず質問だけなので、静的な 2 次元の点集合に対する長方形の XOR を kd 木で
// 取るか 2 次元セグメント木で取るかを比べる。構築も計測区間に入れる。
// テストデータは HackerRank から手で取り込む (source = manual)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-2d.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<array<int, 3>> pts(n);
  for (auto &e : pts) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 4>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d %d", &e[0], &e[1], &e[2], &e[3]), 4);

  Solver sol(pts, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
