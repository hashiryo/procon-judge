#pragma once
#include "common.hpp"
#include "_shared/seqtree/persistent_seq.hpp"

// 永続の列の木の試作 (problems/_shared/seqtree/persistent_seq.hpp、値を葉に持つ重み平衡木) を、永続しない形で使ったもの。2 本の列は
// 同じ置き場を共有して切り貼りする。
struct RangeSumTi : RangeSum {
  static T ti() { return 0; }
};
using Solver = FishSolver<PersistentSeq<RangeSumTi, true, false>>;
