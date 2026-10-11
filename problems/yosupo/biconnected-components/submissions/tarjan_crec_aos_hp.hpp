#pragma once
// tarjan_crec_hp で、剥がしの次数と XOR を 1 つの配列に並べて置き、1 頂点の読み書きを 1 本のキャッシュラインで済ませる。核は _bcc2.hpp。
#include "_bcc2.hpp"
using Solver= bcc2::Solver<2, 1, true>;
