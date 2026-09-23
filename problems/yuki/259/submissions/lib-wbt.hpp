#pragma once
#include "common.hpp"
#include "mylib/data_structure/WeightBalancedTree.hpp"

// 重み平衡木。部分木の大きさの比で平衡を保つ。乱数も償却も使わず、最悪 O(log N)。
using Solver = FishSolver<WeightBalancedTree<RangeSum, true>>;
