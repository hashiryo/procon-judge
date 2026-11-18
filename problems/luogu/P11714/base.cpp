// 洛谷 P11714 / UOJ 37 (主旋律)。n 頂点 m 辺の有向グラフで、辺の部分集合のうち残しても強連結なものの数 mod 1e9+7。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<pair<int, int>> &es);  // 辺は 0 始まりの (a, b) で a → b。構築。計測区間の外。
//     void run();                                         // ここだけ測る。
//     u32 answer() const;                                 // 答え (1e9+7 で割った余り)
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<pair<int, int>> es(m);
  for (auto &[a, b] : es) {
    must_scan(scanf("%d %d", &a, &b), 2);
    --a, --b;
  }

  Solver sol(n, es);
  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%u\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
