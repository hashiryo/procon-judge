#pragma once
// noi に、なめる前の Padberg-Rinaldi の 2 つの条件 (重みが上界以上、2 c > min(deg)) での縮約を足したもの。
#include "_shared/gmincut/noi.hpp"
#define GMC_CORE gmc::noi<1>
#include "_shared/gmincut/solver.hpp"
