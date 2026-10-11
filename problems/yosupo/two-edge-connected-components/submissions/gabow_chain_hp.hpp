#pragma once
// 剥がしたあとの 2-core で、次数 2 の頂点が続く鎖を 16 本ずつ交互に歩いて 1 本の辺に縮め、残った核 (次数 3 以上の頂点と、次数 2 の頂点のうち 32 個に 1 個ほど) だけを記録の形の path-based で分ける。核は _tecc3.hpp。
#include "_tecc3.hpp"
using Solver= tecc3::Solver<5, 16>;
