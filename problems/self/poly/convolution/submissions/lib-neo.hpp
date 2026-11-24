#pragma once
// NeoLibrary の convolve_naive (neo/base/core/convolve/naive.hpp)。
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/naive.hpp"
inline vector<u32> run(const vector<u32>& a, const vector<u32>& b) {
 using Z= ZMod<998244353>;
 vector<Z> A(a.size()), B(b.size());
 for(size_t i= 0; i < a.size(); ++i) A[i]= Z::raw(a[i]);
 for(size_t i= 0; i < b.size(); ++i) B[i]= Z::raw(b[i]);
 const auto C= convolve_naive(A, B);
 vector<u32> c(C.size());
 for(size_t i= 0; i < C.size(); ++i) c[i]= C[i].val();
 return c;
}
