#pragma once
#include "common.hpp"
#include "mylib/algebra/Rational.hpp"

// 有理数で座標を持つ。交点が正確に出るので整列の順序が狂わない。分子と分母を
// 128 ビットで持たないと桁が溢れる。
using Solver = GeoSolver<Rational<__int128_t>>;
