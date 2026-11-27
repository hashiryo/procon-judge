#pragma once
// NeoLibrary の interpolate_iota (neo/algo/compute/poly/lagrange.hpp)。
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
inline u32 run(const vector<u32>& y, u64 x) {
 using Z= ZMod<998244353>;
 vector<Z> Y(y.size());
 for(size_t i= 0; i < y.size(); ++i) Y[i]= Z::raw(y[i]);
 return lagrange::interpolate_iota(Y, x).val();
}
