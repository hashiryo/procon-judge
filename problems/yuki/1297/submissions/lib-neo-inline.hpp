#pragma once
#include <algorithm>
#include "common.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// common.hpp の StatueSolver<Lines> の包みを通さず、run() の中で LiChaoTree を 2 つ直接使う形。
// 左の集合は x = -2 a_i - 2 c i、右の集合は x = i で問う。ライブラリの速さを包み抜きで測る提出。
struct Solver {
  int n;
  i64 c;
  vector<array<i64, 2>> ab;
  i64 ans = 0;

  Solver(int n, i64 c, const vector<array<i64, 2>> &ab) : n(n), c(c), ab(ab) {}

  void run() {
    i64 lo = 0, hi = 0;
    for (i64 i = 1; i <= n; ++i) {
      const i64 x = -2 * ab[i - 1][0] - 2 * c * i;
      if (i == 1) lo = hi = x;
      else lo = std::min(lo, x), hi = std::max(hi, x);
    }
    LiChaoTree<i64> left(lo, hi + 1, n + 1), right(1, n + 1, n + 1);
    left.add_line(0, 0);
    for (i64 i = 1;; ++i) {
      const i64 a = 2 * ab[i - 1][0], b = 2 * ab[i - 1][1], cc = 2 * c * i;
      right.add_line(a - cc, left.query(-a - cc) + cc * (i - 1) + b);
      if (i == n) break;
      left.add_line(i, right.query(i) + cc * (i + 1));
    }
    ans = (right.query(n) + (i64)(n + 1) * n * c) / 2;
  }

  i64 answer() const { return ans; }
};
