#pragma once
// NeoLibrary の BitMatrix (neo/linalg/BitMatrix.hpp) の det。この問題で書き比べた gf2_det_blk16 を写し、掃き出しの要を探す手順を書き直したもの。
#include "../common.hpp"
#include "neo/linalg/BitMatrix.hpp"
inline int run(int, const vector<string>& a) { return BitMatrix(a).det(); }
