#pragma once
#include "common.hpp"
#include "neo/base/ds/seq/Sequence.hpp"

// NeoLibrary の neo/base/ds/seq/Sequence.hpp (塊の形)。2 本の列は同じ置き場を共有して切り貼りする。
struct RangeSumTi : RangeSum {
  static T ti() { return 0; }
};
using Solver = FishSolver<Sequence<RangeSumTi, true>>;
