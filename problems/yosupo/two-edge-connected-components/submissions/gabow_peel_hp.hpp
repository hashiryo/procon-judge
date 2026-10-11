#pragma once
// DFS の前に次数 1 以下の頂点を、次数と隣接の XOR だけで剥がし、残りの 2-core を path-based (Gabow) の DFS で分ける。 隣接は CSR、作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _tecc.hpp。
#include "_tecc.hpp"
using Solver= tecc::Solver<true>;
