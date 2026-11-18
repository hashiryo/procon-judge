#pragma once
// NeoLibrary の pow_table (neo/number_theory/pow_table.hpp) と sum_exp_poly_limit (neo/polynomial/lagrange.hpp)。この問題で書き
// 比べた se1 を、法を定数にしない形で写したもの。
#include "neo/algebra/ZMod.hpp"
#include "neo/number_theory/pow_table.hpp"
#include "neo/polynomial/lagrange.hpp"
inline u32 run(u32 r, u32 d) {
 using Z= ZMod<998244353>;
 return sum_exp_poly_limit(pow_table<Z>(int(d), d), Z(r)).val();
}
