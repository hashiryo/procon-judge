#pragma once
// tarjan_chain_hp で、核の CSR を組むときの番兵を 8 つに回し、鎖の頂点の計数の整列を記録の番号の余りで 4 つの数え口に分ける (同じ番地の読み書きが直列につながらない)。核は _bcc3.hpp。
#include "_bcc3.hpp"
using Solver= bcc3::Solver<5, 16, true>;
