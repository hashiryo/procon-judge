#pragma once
#include <algorithm>
#include "common.hpp"
#include "_shared/lines/lichao_fn.hpp"

// lichao-range と同じ手順を、載せる関数の型をテンプレートで受ける核 (LiChaoRangeFn) に直線 (LineFn) を渡して動かす。
// 最大なので、傾きと切片と答えの符号を反転して最小で持つ。
struct Lines {
  LiChaoRangeFn<LineFn> t;

  explicit Lines(const vector<i64> &xs)
      : t(*std::min_element(xs.begin(), xs.end()), *std::max_element(xs.begin(), xs.end()) + 1, (int)xs.size() + 1) {}

  void insert(i64 a, i64 b) { t.add(LineFn{-a, -b}); }

  i64 query(i64 x) const { return -t.query(x); }
};

using Solver = BallSolver<Lines>;
