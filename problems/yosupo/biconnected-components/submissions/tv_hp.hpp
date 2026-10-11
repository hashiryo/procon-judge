#pragma once
// DFS を使わない Tarjan–Vishkin。BFS の森の上で、行きがけ順の番号と部分木の大きさ、部分木から届く番号の最小と最大を、BFS の順と逆順の走査で求め、木の辺を union-find でつなぐ。隣接は CSR、作業領域は 2 MB 境界の 1 本の領域で huge page を頼む。核は _bcc.hpp。
#include "_bcc.hpp"
using Solver= bcc::Solver<false, 2>;
