#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_Dynamic.hpp"

// 永続な動的セグメント木。更新のたびに根までの経路を作り直すので、高さぶんの
// ノードが 1 頂点につき積み上がる。
using Solver = PathSolver<SegmentTree_Dynamic<CountSum, true>>;
