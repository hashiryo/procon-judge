#pragma once
// snap4 の判定を、分岐なしの contains_min (_basis.hpp) にしたもの。
#include "_snap.hpp"
struct Solver: SnapSolver<4, true> {
 using SnapSolver<4, true>::SnapSolver;
};
