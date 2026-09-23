#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_Patricia.hpp"

// 永続なパトリシア木。枝分かれの無い経路を 1 ノードに畳むので、作り直す
// ノードの数が減る。
using Solver = SeqSolver<SegmentTree_Patricia<RangeMin, true>>;
