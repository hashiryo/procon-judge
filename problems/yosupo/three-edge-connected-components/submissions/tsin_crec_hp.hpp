#pragma once
// tsin_peel_hp の DFS を、頂点ごとの状態を 32 byte の構造体 1 つにまとめた形と、記録 {CSR の始まり, 終わり, 最初の 2 つの隣接} で回し、次数 2 の頂点では親でない方の隣接だけを見る。核は _t3.hpp。
#include "_t3.hpp"
using Solver= t3::Solver<1>;
