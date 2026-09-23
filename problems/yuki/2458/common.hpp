#pragma once
// ライブラリを使う提出が共有するもの。DP はどの実装でも同じなので、ここに置く。
// 比べたいのは直線の集合の最大値を取る道具。
#include <algorithm>
#include "pj.hpp"

constexpr i64 INF = 1e18;

// Lines は次を実装する (最大化)。
//   void insert(i64 a, i64 b);  // 直線 y = a x + b を足す
//   i64 query(i64 x) const;     // x での最大値
//
// 残すボールを左から順に決める。1 つ前に残したボール j の次に i を残すとエネルギー
// が Q_j Q_i 増えるので、dp[i] = max_j dp[j] + Q_j Q_i は傾き Q_j 切片 dp[j] の直線
// を x = Q_i で評価した最大値。最初に残すボールの前には何も無いので、直線 y = 0 を
// 先に入れておく。
template <class Lines> struct BallSolver {
  vector<i64> q;
  i64 ans = 0;

  explicit BallSolver(const vector<i64> &q) : q(q) {}

  void run() {
    const int n = (int)q.size();
    Lines lines;
    lines.insert(0, 0);
    ans = -INF;
    for (int i = 0;;) {
      const i64 dp = lines.query(q[i]);
      ans = std::max(ans, dp);
      if (++i == n) break;
      lines.insert(q[i - 1], dp);
    }
  }

  i64 answer() const { return ans; }
};
