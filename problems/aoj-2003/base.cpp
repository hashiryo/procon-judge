// https://onlinejudge.u-aizu.ac.jp/problems/2003
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const array<i64, 4> &ab,            // 線分 AB
//            const vector<array<i64, 6>> &segs); // {x1, y1, x2, y2, o, l}
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 1 つの入力に T 個のデータセットが入っているので、ハーネスが全部読んでから
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
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<array<i64, 4>> abs_(t);
  vector<vector<array<i64, 6>>> datasets(t);
  for (int i = 0; i < t; ++i) {
    auto &ab = abs_[i];
    must_scan(scanf("%lld %lld %lld %lld", &ab[0], &ab[1], &ab[2], &ab[3]), 4);
    int n;
    must_scan(scanf("%d", &n), 1);
    datasets[i].resize(n);
    for (auto &s : datasets[i])
      must_scan(scanf("%lld %lld %lld %lld %lld %lld", &s[0], &s[1], &s[2], &s[3],
                      &s[4], &s[5]),
                6);
  }

  vector<i64> ans;
  ans.reserve(t);

  auto t0 = chrono::steady_clock::now();
  for (int i = 0; i < t; ++i) {
    Solver s(abs_[i], datasets[i]);
    s.run();
    ans.push_back(s.answer());
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
