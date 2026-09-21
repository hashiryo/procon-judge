#pragma once
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// Splay 木。触った節点を根まで持ち上げるので、分割と結合は根の付け替えで済む。
// 償却 O(log N)。
using Solver = FishSolver<SplayTree<RangeSum, true>>;
