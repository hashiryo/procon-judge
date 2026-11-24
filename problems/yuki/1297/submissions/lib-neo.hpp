#pragma once
#include <algorithm>
#include "common.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTree。範囲は問いの x の最小と最大から決める。
struct Lines {
  LiChaoTree<i64> t;

  explicit Lines(const vector<i64> &xs)
      : t(*std::min_element(xs.begin(), xs.end()), *std::max_element(xs.begin(), xs.end()) + 1, (int)xs.size() + 1) {}

  void insert(i64 a, i64 b) { t.add_line(a, b); }

  i64 query(i64 x) const { return t.query(x); }
};

using Solver = StatueSolver<Lines>;
