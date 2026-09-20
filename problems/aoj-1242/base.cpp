// https://onlinejudge.u-aizu.ac.jp/problems/1242
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<i64, 2>> &ps);  // 多角形の頂点
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 1 つの入力に複数のデータセットが入っているので、ハーネスが全部読んでから
// データセットごとに Solver を作る。計測区間は全部の合計。
//
// 座標は整数。比べたいのは座標の型 (浮動小数点数と有理数) なので、解法は
// 1 つにして提出は型を選ぶだけにしてある。切り上げと切り捨てを誤るとそのまま
// 答えが変わるので、誤差の効き方がはっきり出る問題になっている。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-double.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  vector<vector<array<i64, 2>>> datasets;
  for (;;) {
    int n;
    must_scan(scanf("%d", &n), 1);
    if (n == 0) break;
    vector<array<i64, 2>> ps(n);
    for (auto &p : ps) must_scan(scanf("%lld %lld", &p[0], &p[1]), 2);
    datasets.push_back(std::move(ps));
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
