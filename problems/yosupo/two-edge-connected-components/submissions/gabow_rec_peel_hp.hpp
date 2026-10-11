#pragma once
// 次数 1 以下の頂点を、次数と隣接の XOR だけで剥がしてから、2-core を、頂点ごとの記録 {CSR の始まり, 終わり, 最初の 2 つの隣接} で path-based の DFS で分ける。 作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _tecc.hpp。
#include "_tecc.hpp"
using Solver= tecc::Solver<true, 3>;
