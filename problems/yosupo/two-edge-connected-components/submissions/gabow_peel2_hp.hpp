#pragma once
// gabow_peel_hp の剥がしと CSR を組むところを分岐なしにし、剥がしで待ち行列の先の頂点を先読みする。DFS は同じ CSR の path-based。核は _tecc2.hpp。
#include "_tecc2.hpp"
using Solver= tecc2::Solver<1, 0>;
