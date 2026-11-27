#pragma once
#include "common.hpp"
#include "neo/algo/optimize/line/MonotoneCHT.hpp"

// common.hpp の FrogSolver<Lines> の包みを通さず、run() の中で MonotoneCHT を直接使う形。参照実装と同じく、
// DP のループと直線の操作を 1 つの関数に書く。包みを通す代金を測るための提出。
struct Solver {
  i64 c;
  vector<i64> h;
  i64 ans = 0;

  Solver(i64 c, const vector<i64> &h) : c(c), h(h) {}

  void run() {
    const int n = (int)h.size();
    MonotoneCHT<i64> lines;
    i64 dp = 0;
    lines.add_line(-2 * h[0], h[0] * h[0]);
    for (int i = 1; i < n; ++i) {
      dp = lines.query_monotone(h[i]) + h[i] * h[i] + c;
      lines.add_line(-2 * h[i], dp + h[i] * h[i]);
    }
    ans = dp;
  }

  i64 answer() const { return ans; }
};
