#pragma once
// 次数 1 の頂点を、次数と隣接の XOR だけで剥がしてから、2-core を Tarjan–Vishkin (BFS の森、行きがけ順の番号、部分木から届く番号の最小と最大、union-find) で分ける。隣接は 2-core の CSR、作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _bcc.hpp。
#include "_bcc.hpp"
using Solver= bcc::Solver<true, 2>;
