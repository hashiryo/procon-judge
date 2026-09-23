// https://judge.yosupo.jp/problem/assignment
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &a);      // a は N x N を行優先で並べたもの
//     void run();                               // 最小費用の割り当てを求める
//     i64 cost() const;                         // sum a[i][p[i]]
//     const vector<int> &assignment() const;    // p[i]
//   };
//
// グラフを組むところは run() の中に置く。二部グラフの重み最大マッチングと
// 最小費用流では組み方が違うので、そこも比較の対象になる。答えは一意でない
// ので判定はチェッカに任せる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-weighted-matching.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> a = read_ints(n * n);

  Solver sol(n, a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  string out = to_string(sol.cost());
  out += '\n';
  const auto &p = sol.assignment();
  for (int i = 0; i < n; ++i) {
    out += to_string(p[i]);
    out += (i + 1 == n ? '\n' : ' ');
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
