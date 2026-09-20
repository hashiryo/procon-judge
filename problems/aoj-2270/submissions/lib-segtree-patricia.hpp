#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_Patricia.hpp"

// 永続なパトリシア木。枝分かれの無い経路を 1 ノードに畳むので、値が疎なほど
// 作り直すノードが減る。
using Solver = PathSolver<SegmentTree_Patricia<CountSum, true>>;
