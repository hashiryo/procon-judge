#pragma once
// Hopcroft–Tarjan の反復の DFS を、頂点ごとの記録 {CSR の始まり, 終わり, 最初の 2 つの隣接} で回し、low はフレームに持つ。作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _bcc.hpp。
#include "_bcc.hpp"
using Solver= bcc::Solver<false, true>;
