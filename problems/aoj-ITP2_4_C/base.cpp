// https://onlinejudge.u-aizu.ac.jp/courses/lesson/8/ITP2/all/ITP2_4_C
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);      // 構築。計測区間の外。
//     void swap_ranges(int b, int e, int t);      // [b, e) と [t, t+e-b) を入れ替える
//     vector<i64> dump();                         // 最後の列。計測区間の外。
//   };
//
// 2 つの区間は重ならない。構築と取り出しはどの実装も O(N) なので外に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-rbst.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> a = read_ints(n);
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  Solver s(a);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) s.swap_ranges(e[0], e[1], e[2]);
  auto t1 = chrono::steady_clock::now();

  print_all(s.dump(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
