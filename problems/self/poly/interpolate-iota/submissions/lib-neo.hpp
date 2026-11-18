#pragma once
// NeoLibrary の interpolate_iota (neo/polynomial/lagrange.hpp)。
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/lagrange.hpp"
inline u32 run(const vector<u32>& y, u64 x) {
 using Z= ZMod<998244353>;
 vector<Z> Y(y.size());
 for(size_t i= 0; i < y.size(); ++i) Y[i]= Z::raw(y[i]);
 return interpolate_iota(Y, x).val();
}
