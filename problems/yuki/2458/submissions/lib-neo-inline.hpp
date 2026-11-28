#pragma once
#include <algorithm>
#include "common.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// common.hpp の BallSolver<Lines> の包みを通さず、run() の中で LiChaoTree を直接使う形。
// 範囲は問いの x の最小と最大から決める。ライブラリの速さを包み抜きで測る提出。
struct Solver {
  vector<i64> q;
  i64 ans = 0;

  explicit Solver(const vector<i64> &q) : q(q) {}

  void run() {
    const int n = (int)q.size();
    const i64 lo = *std::min_element(q.begin(), q.end()), hi = *std::max_element(q.begin(), q.end());
    LiChaoTree<i64, true> lines(lo, hi + 1, n + 1);
    lines.add_line(0, 0);
    ans = -INF;
    for (int i = 0;;) {
      const i64 dp = lines.query(q[i]);
      ans = std::max(ans, dp);
      if (++i == n) break;
      lines.add_line(q[i - 1], dp);
    }
  }

  i64 answer() const { return ans; }
};
