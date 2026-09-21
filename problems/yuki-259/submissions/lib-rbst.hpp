#pragma once
#include "common.hpp"
#include "mylib/data_structure/RandomizedBinarySearchTree.hpp"

// 乱択平衡二分木。結合と分割で根を乱数で選ぶので、回転が要らない。期待 O(log N)。
using Solver = FishSolver<RandomizedBinarySearchTree<RangeSum, true>>;
