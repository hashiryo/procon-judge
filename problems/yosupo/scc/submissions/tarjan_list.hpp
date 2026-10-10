#pragma once
// 反復の Tarjan (訪問順と lowlink を頂点ごとの配列で持つ)。隣接は辺の連結リスト。作業領域は配列ごとに malloc する。核は _scc.hpp。
#include "_scc.hpp"
using Solver= scc::Solver<scc::Tarjan, scc::List, false>;
