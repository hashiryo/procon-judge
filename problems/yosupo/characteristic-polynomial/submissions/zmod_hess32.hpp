#pragma once
// zmod_hess64 の行列を u32 で持つ版 (_hess32.hpp)。相似変換の手順ごとの読み書きを半分にする。法は 998244353。
#include "_hess32.hpp"
inline vector<u32> run(int n, const vector<vector<u32>>& M) { return hess32::charpoly(n, M); }
