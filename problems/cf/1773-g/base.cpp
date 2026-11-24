// Codeforces 1773G (Game of Questions)。Genie (1 人目) が最後まで残る確率。
//
// 提出は次を実装する。q[i] の bit j は、質問 i に人 j (0 から。Genie は 0) が正解するとき 1。
//   struct Solver {
//     Solver(int n, int m, const vector<u32> &q);  // 構築。計測区間の外。
//     void run();                                   // ここだけ測る。
//     double answer() const;                        // 確率
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<u32> q(n, 0);
  char buf[32];
  for (int i = 0; i < n; ++i) {
    must_scan(scanf("%31s", buf), 1);
    for (int j = 0; j < m; ++j)
      if (buf[j] == '1') q[i] |= 1u << j;
  }

  Solver sol(n, m, q);
  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%.16f\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
