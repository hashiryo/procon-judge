#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_Dynamic.hpp"

// 永続な動的セグメント木。添字の空間を 2 冪で取って、触ったところだけノードを
// 作る。更新のたびに根までの経路を作り直す。
using Solver = SeqSolver<SegmentTree_Dynamic<RangeMin, true>>;
