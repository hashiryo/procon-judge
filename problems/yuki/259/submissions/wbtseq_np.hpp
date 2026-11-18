#pragma once
#include "common.hpp"
#include "_shared/seqtree/wbt_seq.hpp"

// 今の Library の WeightBalancedTree の中身を写した列の木の試作 (problems/_shared/seqtree/wbt_seq.hpp) を、永続しない形
// (PERSIST = false) で使ったもの。split で外れた節と消した葉を使い回す。
struct RangeSumTi : RangeSum {
  static T ti() { return 0; }
};
using Solver = FishSolver<WbtSeq<RangeSumTi, true, false>>;
