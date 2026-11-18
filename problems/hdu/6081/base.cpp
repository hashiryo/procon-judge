// HDU 6081 度度熊的王国战略。若干組のテストデータで、n 頂点 (n ≤ 3000) m 本 (m ≤ 10^5、重み 1 以上 1000 以下) の無向グラフの
// 全域最小カット (つながっていなければ 0) を答える。辺の両端は同じでもよく、同じ組を結ぶ辺が何本あってもよい。
//
// 提出は luogu-P5632 と同じものを実装する (_shared/gmincut/solver.hpp)。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &e);  // 無向辺 {u, v, 重み}。頂点は 0 始まり
//     i64 run();                                      // 全域最小カットの値
//   };
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
