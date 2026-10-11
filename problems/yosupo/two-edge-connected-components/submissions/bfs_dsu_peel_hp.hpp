#pragma once
// 次数 1 以下の頂点を、次数と隣接の XOR だけで剥がしてから、2-core を BFS の全域森と union-find で分ける。隣接は 2-core の CSR。 作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _tecc.hpp。
#include "_tecc.hpp"
using Solver= tecc::Solver<true, 2>;
