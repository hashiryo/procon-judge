#pragma once
// lowlink (Tarjan) の反復の DFS で二辺連結成分を求める。 隣接は CSR、作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _tecc.hpp。
#include "_tecc.hpp"
using Solver= tecc::Solver<false, 1>;
