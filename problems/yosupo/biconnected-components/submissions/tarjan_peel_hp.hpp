#pragma once
// DFS の前に次数 1 の頂点を、次数と隣接の XOR だけで剥がし (剥がした頂点と残りの隣接が 1 つの成分)、残りの 2-core を Hopcroft–Tarjan の DFS で分ける。隣接は 2-core の CSR、作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _bcc.hpp。
#include "_bcc.hpp"
using Solver= bcc::Solver<true>;
