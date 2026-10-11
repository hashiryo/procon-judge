#pragma once
// tarjan_peel_hp の剥がしと CSR を組むところを分岐なしにし、剥がしで待ち行列の先の頂点を先読みする。DFS は同じ CSR の Hopcroft–Tarjan。核は _bcc2.hpp。
#include "_bcc2.hpp"
using Solver= bcc2::Solver<1, 0>;
