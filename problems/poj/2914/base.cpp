// POJ 2914 Minimum Cut。複数のテストケースで、N 頂点 (2 ≤ N ≤ 500) の無向グラフ (頂点は 0 始まり、行ごとに A と B を結ぶ辺が
// C 本) の全域最小カットの辺の本数を答える。つながっていなければ 0。
//
// 提出は luogu-P5632 と同じものを実装する (_shared/gmincut/solver.hpp)。
// 入力を全部読んだあと、組ごとの構築と run の和を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  vector<pair<int, vector<array<i64, 3>>>> tests;
  int n, m;
  while (scanf("%d %d", &n, &m) == 2) {
    vector<array<i64, 3>> e(m);
    for (auto &[u, v, w] : e) must_scan(scanf("%lld %lld %lld", &u, &v, &w), 3);
    tests.emplace_back(n, std::move(e));
  }
  vector<i64> ans;
  ans.reserve(tests.size());
  auto t0 = chrono::steady_clock::now();
  for (auto &[k, e] : tests) {
    Solver sol(k, e);
    ans.push_back(sol.run());
  }
  auto t1 = chrono::steady_clock::now();
  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
