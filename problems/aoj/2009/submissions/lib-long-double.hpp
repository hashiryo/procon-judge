#pragma once
#include "common.hpp"

// 浮動小数点数で座標を持つ。1 演算が速い代わりに、交点の一致判定に誤差が乗る。
using Solver = GeoSolver<long double>;
