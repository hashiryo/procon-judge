#pragma once
// noi_pr を、今のグラフが密 (辺の数 × 20 が頂点の数の 2 乗以上) になったら隣接行列で回す形 (noi_dense) に移すもの。
#include "_shared/gmincut/noi.hpp"
#define GMC_CORE gmc::noi<1, true>
#include "_shared/gmincut/solver.hpp"
