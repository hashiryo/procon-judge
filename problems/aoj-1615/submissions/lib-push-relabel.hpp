#pragma once
#include "common.hpp"

// 頂点に高さを付けて余剰を押し出す。増加路を探さない。
using Solver = FlowSolver<PushRelabel<i64>>;
