#pragma once
#include "common.hpp"
#include "mylib/data_structure/WeightBalancedTree.hpp"

// 永続な重み平衡木。添字の空間ではなく列そのものを持つので、木の高さが
// log N で収まる。永続にするぶんノードを静的な配列から多めに取る。
using Solver = SeqSolver<WeightBalancedTree<RangeMin, false, true>>;
