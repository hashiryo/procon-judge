#pragma once
#include "common.hpp"

// 浮動小数点数で座標を持つ。交点を整列するので、誤差があると順序が入れ替わる。
using Solver = GeoSolver<long double>;
