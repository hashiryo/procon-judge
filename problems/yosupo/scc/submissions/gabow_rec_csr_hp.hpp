#pragma once
// path-based (Gabow)。頂点ごとの記録に鍵と最初の隣接を並べ (8 byte)、残りの隣接は CSR。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _shared/scc/scc_rec.hpp。
#include "_shared/scc/scc_rec.hpp"
using Solver= scc::Solver<scc::GabowRecDFS, scc::RecCSR, true>;
