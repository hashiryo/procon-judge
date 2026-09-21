#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized_Undoable.hpp"

// 巻き戻せるポテンシャル付き Union-Find。履歴を残すために経路圧縮ができない
// ので、根まで登る距離が O(log N) になる。この問題では巻き戻しを使わないので、
// 経路圧縮を捨てたぶんがそのまま差になる。
using Solver = PathQuerySolver<UnionFind_Potentialized_Undoable<Nimber>>;
