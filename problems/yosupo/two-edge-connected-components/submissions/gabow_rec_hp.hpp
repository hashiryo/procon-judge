#pragma once
// path-based (Gabow) の DFS を、頂点ごとの記録 {CSR の始まり, 終わり, 最初の 2 つの隣接} で回す。 作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _tecc.hpp。
#include "_tecc.hpp"
using Solver= tecc::Solver<false, 3>;
