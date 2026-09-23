#pragma once
#include "common.hpp"

// 頂点に高さを付けて、超過した流れを低い方へ押し出す。
using Solver = TourSolver<PushRelabel<i64>>;
