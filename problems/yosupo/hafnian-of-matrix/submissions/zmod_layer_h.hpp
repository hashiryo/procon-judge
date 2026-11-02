#pragma once
// zmod_layer_c の層ごとの DP で、掛ける列を配る先の列だけに詰める版 (_layer_h.hpp)。組を下の w/2 個と上の残りに分け、上の組の
// 部分集合ごとに、使う列と行だけを並べた隣接行列を作り直す。法は 998244353。
#include "_layer_h.hpp"
inline u32 run(int N, const vector<vector<u32>>& a) { return hafnian_layer_h::hafnian(N, a); }
