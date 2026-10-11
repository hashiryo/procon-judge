#pragma once
// gabow_cpeel_hp の DFS を、頂点ごとの記録 {CSR の始まり, 終わり, 最初の 2 つの隣接} で回し、次数 2 の頂点は親でない方の隣接だけを見る。核は _tecc2.hpp。
#include "_tecc2.hpp"
using Solver= tecc2::Solver<>;
