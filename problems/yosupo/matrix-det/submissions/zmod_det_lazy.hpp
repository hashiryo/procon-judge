#pragma once
// NeoLibrary の ZMod<998244353> で、行を u64 で持って更新を 8 回ごとに畳みながら行列式を掃き出す (_zmod_det.hpp の det_lazy、試作)。
#include "_zmod_det.hpp"
inline u32 run(int, const vector<vector<u32>>& a) { return det_lazy<ZMod<998244353>>(a).val(); }
