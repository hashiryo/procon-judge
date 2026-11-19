#pragma once
#include "common.hpp"
#include "_shared/lines/lichao.hpp"

// 問いの x を先に受け取り、座標を詰めた Li Chao 木 (LiChaoPoints) に入れる。問いは二分探索で葉を引いてから根へ上る。
// 最大なので、傾きと切片と答えの符号を反転して最小で持つ。
struct Lines {
  LiChaoPoints t;

  explicit Lines(const vector<i64> &xs) : t(xs) {}

  void insert(i64 a, i64 b) { t.add_line(-a, -b); }

  i64 query(i64 x) const { return -t.query(x); }
};

using Solver = BallSolver<Lines>;
