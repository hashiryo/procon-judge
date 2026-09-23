#pragma once
#include "common.hpp"

// 浮動小数点数で座標を持つ。切り上げと切り捨てに許容誤差を入れないと、
// 格子点の数え方が 1 ずれる。
using Solver = GeoSolver<double>;
