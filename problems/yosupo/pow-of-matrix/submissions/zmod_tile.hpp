#pragma once
// 二分累乗。積は _tile.hpp の 4 行 × 8 列のタイルの核で、値は Montgomery の表現のまま掛け続ける。
#include "_tile.hpp"
inline vector<u32> run(int N, u64 K, const vector<u32>& a) {
 const int w= (N + 7) / 8 * 8;
 return tile::store(N, w, tile::pow(tile::load(N, w, a), K, w));
}
