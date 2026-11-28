// neo の mod_tetration (法を実行時に受ける版) で解く。
#pragma once
#include "neo/algo/compute/nt/tetration.hpp"

struct Solver {
 unsigned solve(unsigned a, unsigned b) { return (unsigned)mod_tetration(a, b, 998244353); }
};
