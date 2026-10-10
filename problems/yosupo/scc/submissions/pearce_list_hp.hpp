#pragma once
// Pearce の省メモリ版 (頂点ごとの配列は 1 本)。隣接は辺の連結リスト。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _shared/scc/scc.hpp。
#include "_shared/scc/scc.hpp"
using Solver= scc::Solver<scc::Pearce, scc::List, true>;
