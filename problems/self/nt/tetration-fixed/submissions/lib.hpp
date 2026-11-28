// 今の Library の mod_tetration (法を実行時に受ける) を使う版 (比較の基準)。
#pragma once
#include "mylib/number_theory/mod_tetration.hpp"

struct Solver {
  unsigned solve(unsigned a, unsigned b) { return (unsigned)mod_tetration(a, b, 998244353); }
};
