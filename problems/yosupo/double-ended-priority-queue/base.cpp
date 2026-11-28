// https://judge.yosupo.jp/problem/double_ended_priority_queue
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<i64> s, vector<int> type, vector<i64> x);  // 入力を覚えるだけ。計測区間の外。
//     void run();                          // ここだけ測る。S の構築と全クエリの処理を行う。
//     const vector<i64> &answer() const;   // タイプ 1, 2 で取り出した値を処理の順に並べたもの。
//   };
// s はクエリより前の初期の多重集合。クエリ i は type[i] が 0 なら x[i] の追加、1 なら最小の
// 要素を 1 つ取り出して出力、2 なら最大の要素を 1 つ取り出して出力。タイプ 1, 2 を処理する
// 直前は S が空でないことが保証される。値は -10^9 以上 10^9 以下。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> s(n);
  for (int i = 0; i < n; ++i) must_scan(scanf("%lld", &s[i]), 1);
  vector<int> type(q);
  vector<i64> x(q, 0);
  for (int i = 0; i < q; ++i) {
    must_scan(scanf("%d", &type[i]), 1);
    if (type[i] == 0) must_scan(scanf("%lld", &x[i]), 1);
  }

  Solver sol(n, s, type, x);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
