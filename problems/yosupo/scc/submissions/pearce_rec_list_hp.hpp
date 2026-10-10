#pragma once
// Pearce の省メモリ版。頂点ごとの記録に鍵と最初の隣接と残りの隣接の連結リストの先頭を並べる (16 byte)。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _scc_rec.hpp。
#include "_scc_rec.hpp"
using Solver= scc::Solver<scc::PearceRecDFS, scc::RecList, true>;
