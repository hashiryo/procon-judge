// CodeChef STR_FUNC (Strange Functions)。sum_i i F[i] G[i] (mod 1e9 + 7)。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<int> &a);  // 構築。計測区間の外。
//     void run();                    // ここだけ測る。
//     u32 answer() const;            // 1e9 + 7 で割った余り
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);

  Solver sol(a);
  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%u\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
