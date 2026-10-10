#pragma once
// path-based (Gabow)。鍵、最初の隣接、残りの隣接の連結リストの先頭を、別々の 4 byte の配列に置く (RecSplit)。DFS の前に入次数 0 の頂点を剥がし、剥がすときに待ち行列の先の頂点を先読みする。入次数は隣接を組む走査の中で数える。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _scc_rec.hpp。
#include "_scc_rec.hpp"
using Solver= scc::Solver<scc::GabowRecTrimPfFused, scc::RecSplit, true>;
