#pragma once
// tarjan_cpeel_hp の DFS を、頂点ごとの記録 {CSR の始まり, 終わり, 最初の 2 つの隣接} で回し、下りた直後は親への辺を番号の比較で飛ばし、次数 2 の頂点は親でない方の隣接だけを見る。核は _bcc2.hpp。
#include "_bcc2.hpp"
using Solver= bcc2::Solver<>;
