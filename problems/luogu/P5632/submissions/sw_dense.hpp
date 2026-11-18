#pragma once
// Stoer-Wagner の方法を隣接行列で回す O(n^3) の形 (_shared/gmincut/sw_dense.hpp)。
#include "_shared/gmincut/sw_dense.hpp"
#define GMC_CORE gmc::sw_dense
#include "_shared/gmincut/solver.hpp"
