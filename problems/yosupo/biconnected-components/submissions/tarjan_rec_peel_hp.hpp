#pragma once
// 次数 1 の頂点を、次数と隣接の XOR だけで剥がしてから、2-core を、頂点ごとの記録 {CSR の始まり, 終わり, 最初の 2 つの隣接} で Hopcroft–Tarjan の DFS で分ける。low はフレームに持つ。作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _bcc.hpp。
#include "_bcc.hpp"
using Solver= bcc::Solver<true, true>;
