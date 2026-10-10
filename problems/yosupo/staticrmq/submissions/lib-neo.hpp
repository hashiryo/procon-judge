#pragma once
// NeoLibrary の RangeMin (neo/data_structure/RangeMin.hpp)。この問題で書き比べた block16_fused を写したもので、
// 表が 1 MB 未満のときは huge page を頼まない分だけが違う。
#include "pj.hpp"
#include "neo/data_structure/RangeMin.hpp"
struct Solver {
 RangeMin rm;
 explicit Solver(const vector<u32>& a): rm(a) {}
 u32 query(int l, int r) const { return rm.min(l, r); }
};
