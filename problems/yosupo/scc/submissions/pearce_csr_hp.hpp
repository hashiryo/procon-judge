#pragma once
// Pearce の省メモリ版 (頂点ごとの配列は 1 本)。隣接は CSR。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _scc.hpp。
#include "_scc.hpp"
using Solver= scc::Solver<scc::Pearce, scc::CSR, true>;
