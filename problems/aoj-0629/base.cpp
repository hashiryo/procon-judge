// https://onlinejudge.u-aizu.ac.jp/challenges/sources/JOI/Final/0629
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 3>> &qs);  // {X, D, L}
//     void run();                         // ここだけ測る。
//     const vector<i64> &answer() const;  // 1 から n までの n 個
//   };
//
// クエリを後ろから見て、区間加算つきのセグメント木の上を二分探索で降りる。
// 探す向きが実装で違うので、載せるモノイドもそれぞれの側で決める。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-max-right.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  Solver sol(n, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
