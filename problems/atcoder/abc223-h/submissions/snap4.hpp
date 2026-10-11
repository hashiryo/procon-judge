#pragma once
// 接頭辞の基底の写しを 4 個おきに持つ (_snap.hpp)。
#include "_snap.hpp"
struct Solver: SnapSolver<4> {
 using SnapSolver<4>::SnapSolver;
};
