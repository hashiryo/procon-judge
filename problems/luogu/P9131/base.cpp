// 洛谷 P9131 ([USACO23FEB] Problem Setting P)。問題の並べ方の数 (mod 1e9 + 7)。
//
// 提出は次を実装する。cnt[S] は、hard と評価した人の集合が S の問題の数 (S は M bit)。
//   struct Solver {
//     Solver(int m, const vector<int> &cnt);  // 構築。計測区間の外。
//     void run();                             // ここだけ測る。
//     u32 answer() const;                     // 1e9 + 7 で割った余り
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<string> s(m);
  {
    static char buf[200005];
    for (auto &t : s) {
      must_scan(scanf("%200000s", buf), 1);
      t = buf;
    }
  }
  vector<int> cnt(size_t(1) << m, 0);
  for (int i = 0; i < n; ++i) {
    int mask = 0;
    for (int j = 0; j < m; ++j)
      if (s[j][i] == 'H') mask |= 1 << j;
    ++cnt[mask];
  }

  Solver sol(m, cnt);
  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%u\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
