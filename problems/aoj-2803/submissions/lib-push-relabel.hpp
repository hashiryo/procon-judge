#pragma once
#include "common.hpp"

// 頂点に高さを付けて余剰を押し出す。増加路を探さないので、密なグラフや流量の大きいグラフで強い。
using Solver = FlowSolver<PushRelabel<i64>>;
