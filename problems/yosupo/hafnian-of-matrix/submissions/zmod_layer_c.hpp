#pragma once
// zmod_layer の頂点ごとの配列を、組 p の頂点なら部分集合の番号から bit p を抜いた番号 (pext) で引いて半分に詰める版
// (_layer.hpp の COMPACT)。N = 38 で配列が 38 MB から 19 MB になる。法は 998244353。
#include "_layer.hpp"
inline u32 run(int N, const vector<vector<u32>>& a) { return hafnian_layer::hafnian<true>(N, a); }
