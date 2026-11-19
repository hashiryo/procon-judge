#pragma once
#include <algorithm>
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// 範囲を渡す Li Chao 木 (LiChaoRange)。範囲は問いの x の最小と最大から決める。節は使う所だけ作り、追加も問いも根から
// 1 本の道を下る。最大なので、傾きと切片と答えの符号を反転して最小で持つ。
struct Lines {
  LiChaoRange t;

  explicit Lines(const vector<i64> &xs)
      : t(*std::min_element(xs.begin(), xs.end()), *std::max_element(xs.begin(), xs.end()) + 1, (int)xs.size() + 1) {}

  void insert(i64 a, i64 b) { t.add_line(-a, -b); }

  i64 query(i64 x) const { return -t.query(x); }
};

using Solver = BallSolver<Lines>;
