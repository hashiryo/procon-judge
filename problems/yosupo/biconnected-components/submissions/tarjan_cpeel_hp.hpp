#pragma once
// tarjan_peel2_hp で、剥がしたあとの 2-core の頂点に 0 から番号を振り直し、DFS の配列をその頂点の数で持つ。核は _bcc2.hpp。
#include "_bcc2.hpp"
using Solver= bcc2::Solver<2, 0>;
