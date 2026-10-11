#pragma once
// 写しを 4 個おきに持ち、問い合わせでは足し直さずに、写しの基底と残りの元を min の形で消して判定する (_snap.hpp の RED)。
#include "_snap.hpp"
struct Solver: SnapSolver<4, false, true> {
 using SnapSolver<4, false, true>::SnapSolver;
};
