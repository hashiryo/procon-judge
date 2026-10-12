#pragma once
// DFS の前に次数 1 以下の頂点を次数と XOR だけで剥がし (剥がした頂点は 1 頂点の成分)、2-core の頂点を詰めた CSR で tsin_hp と同じ DFS を回す。核は _t3.hpp。
#include "_t3.hpp"
using Solver= t3::Solver<false>;
