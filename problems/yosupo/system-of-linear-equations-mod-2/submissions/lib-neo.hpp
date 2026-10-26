#pragma once
// NeoLibrary の BitMatrix (neo/linalg/BitMatrix.hpp) の solve_with_kernel。この問題で書き比べた gf2_sle_blk16 を写し、掃き出しの要を探す手順を書き直したもので、
// 核の基底は、要の行と単位ベクトルを並べた正方行列を転置して行で取り出す。
#include "../common.hpp"
#include "neo/linalg/BitMatrix.hpp"
inline vector<string> run(int, int m, const vector<string>& a, const vector<int>& b) {
 auto r= BitMatrix(a).solve_with_kernel(b);
 if(!r) return {};
 const auto& [x, k]= *r;
 vector<string> res;
 res.reserve(k.height() + 1);
 string& s= res.emplace_back(m, '0');
 for(int j= 0; j < m; ++j) s[j]+= x[j];
 for(auto& t: k.to_strings()) res.push_back(std::move(t));
 return res;
}
