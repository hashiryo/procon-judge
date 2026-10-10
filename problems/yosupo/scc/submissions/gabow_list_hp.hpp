#pragma once
// path-based (Gabow。頂点ごとの配列は 1 本と、根の候補のスタック)。隣接は辺の連結リスト。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _scc.hpp。
#include "_scc.hpp"
using Solver= scc::Solver<scc::Gabow, scc::List, true>;
