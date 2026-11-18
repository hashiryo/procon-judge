// UOJ 80 二分图最大权匹配。男 n_l 人と女 n_r 人 (どちらも 400 以下) と、組める組 m 個 (m ≤ 160000、重み 1 以上 10^9 以下、
// 同じ組は 1 回) が与えられる。重みの和が最大のマッチング (全員が組まなくてよい) の和と、組を 1 つ出す。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int nl, int nr, const vector<array<i64, 3>> &e);  // 組 {男, 女, 重み}。どれも 0 始まり
//     void run();
//     i64 sum() const;                 // 重みの和の最大
//     const vector<int> &mate() const;  // 男ごとの女 (いなければ -1)
//   };
// 入力を読んだあと、構築と run を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int nl, nr, m;
  must_scan(scanf("%d %d %d", &nl, &nr, &m), 3);
  vector<array<i64, 3>> e(m);
  for (auto &[v, u, w] : e) must_scan(scanf("%lld %lld %lld", &v, &u, &w), 3), --v, --u;
  auto t0 = chrono::steady_clock::now();
  Solver sol(nl, nr, e);
  sol.run();
  auto t1 = chrono::steady_clock::now();
  string out = to_string(sol.sum()) + "\n";
  for (int v = 0; v < nl; ++v) out += to_string(sol.mate()[v] + 1), out += v + 1 < nl ? ' ' : '\n';
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
