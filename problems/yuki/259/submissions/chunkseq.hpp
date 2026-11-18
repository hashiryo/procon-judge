#pragma once
#include "common.hpp"
#include "_shared/seqtree/chunk_seq.hpp"

// 列の木の汎用の形 (problems/_shared/seqtree/chunk_seq.hpp、節が値の配列を持つ treap)。2 本の列は同じ置き場を共有して切り貼りする。
struct RangeSumTi : RangeSum {
  static T ti() { return 0; }
};
using Solver = FishSolver<ChunkSeq<RangeSumTi, true>>;
