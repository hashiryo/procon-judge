#pragma once
// path-based (Gabow)。頂点ごとの記録に鍵と最初の隣接を並べ (8 byte)、残りの隣接は CSR。記録を作る走査を分岐にせず、入次数は次数を数える走査で数え、剥がしは peel_pf2、DFS の根の走査は入次数の配列で行う。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _scc_rec.hpp。
#include "_scc_rec.hpp"
using Solver= scc::Solver<scc::GabowRecTrimPf2FusedDs, scc::RecCSRB, true>;
