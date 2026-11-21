#pragma once
// NeoLibrary の interpolate (neo/algo/compute/poly/lagrange.hpp)。
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
inline vector<u32> run(const vector<u32>& x, const vector<u32>& y) {
 using Z= ZMod<998244353>;
 vector<Z> X(x.size()), Y(y.size());
 for(size_t i= 0; i < x.size(); ++i) X[i]= Z::raw(x[i]), Y[i]= Z::raw(y[i]);
 const auto f= interpolate(X, Y);
 vector<u32> c(f.size());
 for(size_t i= 0; i < f.size(); ++i) c[i]= f[i].val();
 return c;
}
