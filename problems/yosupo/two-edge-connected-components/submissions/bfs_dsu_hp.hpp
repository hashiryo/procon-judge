#pragma once
// DFS を使わず、BFS で全域森を作り、木に入らない辺ごとに両端から union-find で道を縮める。隣接は CSR。 作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _tecc.hpp。
#include "_tecc.hpp"
using Solver= tecc::Solver<false, 2>;
