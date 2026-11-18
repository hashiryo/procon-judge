// AtCoder ARC198 E (Monotone OR)。x = 0 から x を (x OR s_i) + 1 に替える操作で x = 2^N に着く列の数 (mod 998244353)。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<int> &s);  // 構築。計測区間の外。
//     void run();                           // ここだけ測る。
//     u32 answer() const;                   // 998244353 で割った余り
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<int> s(m);
  for (auto &x : s) must_scan(scanf("%d", &x), 1);

  Solver sol(n, s);
  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%u\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
