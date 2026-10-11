#pragma once
// 接頭辞の基底の写しを 16 個おきに持つ (_snap.hpp)。
#include "_snap.hpp"
struct Solver: SnapSolver<16> {
 using SnapSolver<16>::SnapSolver;
};
