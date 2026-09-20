#pragma once
#include "common.hpp"

// 浮動小数点数で座標を持つ。1 演算が速い代わりに、比較に誤差が乗る。
// 幾何ライブラリ側が許容誤差で符号を判定している。
using Solver = GeoSolver<long double>;
