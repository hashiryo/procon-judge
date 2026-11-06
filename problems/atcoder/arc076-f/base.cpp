// https://atcoder.jp/contests/arc076/tasks/arc076_d
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int m, const vector<array<int, 2>> &lr);  // lr[i] = {L_i, R_i}
//     void run();
//     int answer() const;  // 足す椅子の数の最小
//   };
//
// 椅子は座標 1 から M に 1 脚ずつあり、人 i は座標が L_i 以下か R_i 以上の椅子に座りたい (0 ≤ L_i < R_i ≤ M + 1)。
// 答えは N から最大マッチングを引いたもの。想定解は貪欲か Hall の定理とセグメント木で、この問題を置いたのは、累積の鎖で
// 区間をまとめた網を最大流で解く力ずくの解き方を測るため (algo-notes の network-flow-brute-force.md)。構築と run を測る。
// lr は計測区間のあとまで生きているので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> lr(n);
  for (auto &[l, r] : lr) must_scan(scanf("%d %d", &l, &r), 2);

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, m, lr);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%d\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
