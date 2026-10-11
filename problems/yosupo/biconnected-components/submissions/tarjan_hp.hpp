#pragma once
// Hopcroft–Tarjan の反復の DFS で二重連結成分を求める。隣接は CSR、作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _bcc.hpp。
#include "_bcc.hpp"
using Solver= bcc::Solver<false>;
