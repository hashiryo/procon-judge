// neo の法を型で固定した mod_tetration<M> (ModIntExp で指数の法をたどる) で解く。
#pragma once
#include "neo/algo/compute/nt/tetration.hpp"

struct Solver {
 unsigned solve(unsigned a, unsigned b) { return (unsigned)mod_tetration<998244353>(a, b); }
};
