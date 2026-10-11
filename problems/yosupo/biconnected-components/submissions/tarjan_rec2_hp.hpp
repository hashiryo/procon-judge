#pragma once
// 剥がさずに、tarjan_crec_hp と同じ記録の形の DFS で分ける。閉路のように剥がれないグラフで、剥がしの分を払わない形。核は _bcc2.hpp。
#include "_bcc2.hpp"
using Solver= bcc2::Solver<0, 1>;
