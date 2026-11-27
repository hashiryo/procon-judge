#pragma once
// NeoLibrary の pow_table (neo/base/core/nt/pow_table.hpp) と sum_exp_poly (neo/algo/compute/poly/lagrange.hpp)。この問題で書き比べた
// se1 を、法を定数にしない形で写したもの。
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/nt/pow_table.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
inline u32 run(u32 r, u32 d, u64 n) {
 using Z= ZMod<998244353>;
 return lagrange::sum_exp_poly(pow_table<Z>(int(d), d), Z(r), n).val();
}
