#pragma once
// NeoLibrary の BitMatrix (neo/linalg/BitMatrix.hpp) の inverse。この問題で書き比べた gf2_inv_blk16 を写し、掃き出しの要を探す手順を書き直したもので、[A | I] の
// I を A の次の語から、要にした順に付け替えて置く。
#include "../common.hpp"
#include "neo/linalg/BitMatrix.hpp"
inline vector<string> run(int, const vector<string>& a) {
 const auto r= BitMatrix(a).inverse();
 return r ? r->to_strings() : vector<string>{};
}
