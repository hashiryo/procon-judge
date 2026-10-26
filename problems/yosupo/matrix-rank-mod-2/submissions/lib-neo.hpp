#pragma once
// NeoLibrary の BitMatrix (neo/linalg/BitMatrix.hpp) の rank。この問題で書き比べた gf2_rank_blk16 を写し、掃き出しの要を探す手順を書き直したもの。
// 行が列より多ければ、BitMatrix::transposed_from で文字列から転置した形で作る (2^24 × 1 でも 1 × 2^24 の行列で済み、行ごとに
// 1 語を置かない)。
#include "../common.hpp"
#include "neo/linalg/BitMatrix.hpp"
inline int run(int n, int m, const vector<string>& a) { return n > m ? BitMatrix::transposed_from(a).rank() : BitMatrix(a).rank(); }
