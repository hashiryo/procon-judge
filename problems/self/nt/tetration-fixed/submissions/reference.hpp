// 期待出力を作る参照実装。今の Library の mod_tetration (λ のような値で指数を落とす) を固定したもの。
// ジェネレータか base.cpp を直したときだけ作り直されるよう、ライブラリ以外は include しない。
#pragma once
#include <cstdint>
#include "mylib/number_theory/mod_tetration.hpp"

struct Solver {
 unsigned solve(unsigned a, unsigned b) { return (unsigned)mod_tetration(a, b, 998244353); }
};
