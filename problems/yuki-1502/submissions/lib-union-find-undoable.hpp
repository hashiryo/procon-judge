#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized_Undoable.hpp"

// 巻き戻せるポテンシャル付き Union-Find。履歴を残すために経路圧縮ができない
// ので、potential を取るたびに根まで O(log N) 登る。この問題では巻き戻しを
// 使わないので、経路圧縮を捨てたぶんがそのまま差になる。
using Solver = CountSolver<UnionFind_Potentialized_Undoable<G>>;
