// https://onlinejudge.u-aizu.ac.jp/problems/2009
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<i64, 4>> &segs);  // {x1, y1, x2, y2}
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 1 つの入力に複数のデータセットが入っているので、ハーネスが全部読んでから
// データセットごとに Solver を作る。計測区間は全部の合計。
//
// 座標は整数。比べたいのは座標の型 (浮動小数点数と有理数) なので、解法は
// 1 つにして提出は型を選ぶだけにしてある。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-long-double.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  vector<vector<array<i64, 4>>> datasets;
  for (;;) {
    int n;
    must_scan(scanf("%d", &n), 1);
    if (n == 0) break;
    vector<array<i64, 4>> segs(n);
    for (auto &s : segs)
      must_scan(scanf("%lld %lld %lld %lld", &s[0], &s[1], &s[2], &s[3]), 4);
    datasets.push_back(std::move(segs));
  }

  vector<i64> ans;
  ans.reserve(datasets.size());

  auto t0 = chrono::steady_clock::now();
  for (auto &ds : datasets) {
    Solver s(ds);
    s.run();
    ans.push_back(s.answer());
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
