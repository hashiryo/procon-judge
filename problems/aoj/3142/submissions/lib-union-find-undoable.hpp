#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized_Undoable.hpp"

// 巻き戻せる重み付き Union-Find。履歴を残すために経路圧縮ができないので、
// 根まで登る距離が O(log N) になる。この問題では巻き戻しを使わない。
using Solver = PotentialSolver<UnionFind_Potentialized_Undoable<i64>>;
