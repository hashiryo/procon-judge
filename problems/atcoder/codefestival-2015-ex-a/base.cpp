// https://atcoder.jp/contests/code-festival-2015-exhibition/tasks/codefestival_2015_ex_a
//
// N 頂点 (N ≤ 5000) M 本 (M ≤ 50000) の有向グラフ (多重辺あり) で、頂点 1 を含み頂点 N を含まない S のうち、S から出る辺の
// 本数が最小のもの (1 から N への最小カット) を考える。2 頂点 c, d が Q 回 (Q ≤ 10^5) 与えられるので、c と d が同じ側に
// なる最小カットがあるか、違う側になる最小カットがあるかを答える。
//
// 提出は次を実装する。
//   struct Solver {
//     // 頂点は 0 始まり。roads は辺 (a, b)、qs は問い (c, d)
//     Solver(int n, const vector<array<int, 2>> &roads, const vector<array<int, 2>> &qs);
//     vector<int> run();  // 問いごとに、bit 0 が同じ側になりうるか、bit 1 が違う側になりうるか
//   };
// 入力を読んだあと、構築と run を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> roads(m);
  for (auto &[a, b] : roads) must_scan(scanf("%d %d", &a, &b), 2), --a, --b;
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 2>> qs(q);
  for (auto &[c, d] : qs) must_scan(scanf("%d %d", &c, &d), 2), --c, --d;
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, roads, qs);
  const vector<int> ans = sol.run();
  auto t1 = chrono::steady_clock::now();
  string out;
  out.reserve(ans.size() * 8);
  for (int x : ans) out += x & 1 ? "YES " : "NO ", out += x & 2 ? "YES\n" : "NO\n";
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
