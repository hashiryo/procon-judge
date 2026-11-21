#pragma once
// NeoLibrary の BitMatrix (neo/base/core/linalg/BitMatrix.hpp) の積。この問題で書き比べた gf2_mul_m4r8 を写したもの。
#include "../common.hpp"
#include "neo/base/core/linalg/BitMatrix.hpp"
inline vector<string> run(int, int, int, const vector<string>& a, const vector<string>& b) { return (BitMatrix(a) * BitMatrix(b)).to_strings(); }
