#pragma once
// path-based (Gabow。頂点ごとの配列は 1 本と、根の候補のスタック)。隣接は辺の連結リスト。作業領域は配列ごとに malloc する。核は _scc.hpp。
#include "_scc.hpp"
using Solver= scc::Solver<scc::Gabow, scc::List, false>;
