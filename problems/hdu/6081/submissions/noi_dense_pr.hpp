#pragma once
// Nagamochi-Ono-Ibaraki の方法を隣接行列で回す形 (_shared/gmincut/noi_dense.hpp) に、Padberg-Rinaldi の 2 つの条件での縮約を足したもの。
#include "_shared/gmincut/noi_dense.hpp"
#define GMC_CORE gmc::noi_dense<true>
#include "_shared/gmincut/solver.hpp"
