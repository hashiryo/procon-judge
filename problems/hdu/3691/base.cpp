// HDU 3691 Nubulsa Expo。複数のテストケース (0 0 0 で終わる) で、N 頂点 (N ≤ 300) M 本 (M ≤ 50000、容量 ≤ 10^6) の連結な
// 無向グラフと入口 S が与えられる。出口 T を選んで S から T への最大流を最小にしたときの値を答える。どの T でも S-T の最小
// カットは全域のカットなので、答えは全域最小カットの値 (S によらない)。
//
// 提出は luogu-P5632 と同じものを実装する (_shared/gmincut/solver.hpp)。入口は渡さない。
// 入力を全部読んだあと、組ごとの構築と run の和を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  vector<pair<int, vector<array<i64, 3>>>> tests;
  int n, m, s;
  while (scanf("%d %d %d", &n, &m, &s) == 3 && (n || m || s)) {
    vector<array<i64, 3>> e(m);
    for (auto &[u, v, w] : e) must_scan(scanf("%lld %lld %lld", &u, &v, &w), 3), --u, --v;
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
