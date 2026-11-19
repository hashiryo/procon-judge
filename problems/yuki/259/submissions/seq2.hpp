#pragma once
#include "common.hpp"
#include "_shared/seqtree/seq2.hpp"

// NeoLibrary の Sequence (塊の形) の手直しの試作 (problems/_shared/seqtree/seq2.hpp)。2 本の列は同じ置き場を共有して切り貼りする。
struct RangeSumTi : RangeSum {
  static T ti() { return 0; }
};
using Solver = FishSolver<Seq2<RangeSumTi, true>>;
