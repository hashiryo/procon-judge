#pragma once
// NeoLibrary の HashMap (neo/data_structure/HashMap.hpp)。この問題で書き比べた lp_hp を写したもので、splitmix64 の最後の
// xorshift を省き、消去と move を足した形。
#include "pj.hpp"
#include "neo/data_structure/HashMap.hpp"
struct Solver {
 HashMap<u64> mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
