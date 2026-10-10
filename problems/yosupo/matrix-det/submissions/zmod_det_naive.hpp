#pragma once
// NeoLibrary の ZMod<998244353> で行列式を掃き出す (_zmod_det.hpp の det_naive、試作)。
#include "_zmod_det.hpp"
inline u32 run(int, const vector<vector<u32>>& a) {
 return run_det(a, [](auto b) { return det_naive(std::move(b)); });
}
