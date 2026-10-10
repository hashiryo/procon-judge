#pragma once
// Kosaraju (順向きと逆向きの隣接を組み、2 回たどる)。隣接は辺の連結リスト。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _shared/scc/scc.hpp。
#include "_shared/scc/scc.hpp"
using Solver= scc::Solver<scc::Kosaraju, scc::List, true>;
