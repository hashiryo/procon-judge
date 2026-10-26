#pragma once
// NeoLibrary の BitMatrix (neo/linalg/BitMatrix.hpp) の rank。この問題で書き比べた gf2_rank_blk16 を写し、掃き出しの要を探す手順を書き直したもので、64 列以下の
// 行列は転置せずに 1 語の行のまま基底に入れ、行が列の 2 倍より多い行列は
// BitMatrix の中で転置してから消す。
#include "../common.hpp"
#include "neo/linalg/BitMatrix.hpp"
inline int run(int, int, const vector<string>& a) { return BitMatrix(a).rank(); }
