#pragma once
#include "common.hpp"
#include "mylib/algebra/Rational.hpp"

// 有理数で座標を持つ。約分と多倍長の掛け算が要るので遅いが、比較が正確なので
// 誤差で答えが変わらない。
using Solver = GeoSolver<Rational<__int128_t>>;
