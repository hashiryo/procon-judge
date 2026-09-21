// https://yukicoder.me/problems/no/421
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<string> &grid);  // 板チョコ。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 幸福度の最大
//   };
//
// 隣り合うホワイトとビターの組をできるだけ多く取る 2 部マッチングで、あとは
// 個数の式。マス目から辺を作るところは common.hpp に置き、マッチングを 2 部
// グラフの最大マッチング、マトロイド交差、重み付きマトロイド交差で取るのを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-bipartite-matching.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<string> grid(n);
  for (auto &s : grid) s = read_token();

  Solver sol(grid);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
