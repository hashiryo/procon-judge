// https://loj.ac/p/117
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int s, int t, const vector<array<int, 4>> &edges);  // {u, v, 下限, 上限}
//     void run();          // 下限と上限を守り、s と t のほかで釣り合う流れのうち、s から t への最小流を求める
//     i64 answer() const;  // 最小流の値。流れが無ければ -1
//   };
//
// 頂点は 0-indexed にして渡す。n は 50003 以下、辺は 125003 本以下で、0 <= 下限 <= 上限 <= 2^31 - 1。多重辺は無い。u と v は違う。流れが無ければ please go home to sleep と出す。
// 構築と run を測る。edges は計測区間のあとまで生きているので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, s, t;
  must_scan(scanf("%d %d %d %d", &n, &m, &s, &t), 4);
  vector<array<int, 4>> edges(m);
  for (auto &e : edges) {
    must_scan(scanf("%d %d %d %d", &e[0], &e[1], &e[2], &e[3]), 4);
    --e[0], --e[1];
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, s - 1, t - 1, edges);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const i64 ans = sol.answer();
  if (ans < 0) puts("please go home to sleep");
  else printf("%lld\n", ans);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
