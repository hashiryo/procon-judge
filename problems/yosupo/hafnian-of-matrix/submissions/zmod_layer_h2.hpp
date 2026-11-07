#pragma once
// zmod_layer_h の層 8 以上の核で、列の向きの和を 5 本ずつ 2 回に分けて回す版 (_layer_h2.hpp)。和と係数と定数を ymm の 16 本に
// 収めて、GCC がループの中でスタックへ退避しないようにする。法は 998244353。
#include "_layer_h2.hpp"
inline u32 run(int N, const vector<vector<u32>>& a) { return hafnian_layer_h2::hafnian(N, a); }
