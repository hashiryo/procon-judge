#pragma once
// NeoLibrary の berlekamp_massey (neo/polynomial/berlekamp_massey.hpp)。この問題で書き比べた bm_avx2 を、法を定数にしない形で
// 写したもの。
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/berlekamp_massey.hpp"
inline vector<u32> run(const vector<u32>& a) {
 using Z= ZMod<998244353>;
 vector<Z> b(a.size());
 for(size_t i= 0; i < a.size(); ++i) b[i]= Z::raw(a[i]);
 const auto c= berlekamp_massey(b);
 vector<u32> r(c.size());
 for(size_t i= 0; i < c.size(); ++i) r[i]= c[i].val();
 return r;
}
