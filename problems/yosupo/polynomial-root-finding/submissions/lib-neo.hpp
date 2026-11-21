#pragma once
// NeoLibrary の poly_roots (neo/algo/compute/poly/poly_roots.hpp)。この問題で書き比べた naive を、法を定数にしない形で写したもの。
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/compute/poly/poly_roots.hpp"
inline vector<u32> run(const vector<u32>& f) {
 using Z= ZMod<998244353>;
 vector<Z> g(f.size());
 for(size_t i= 0; i < f.size(); ++i) g[i]= Z::raw(f[i]);
 const auto rs= poly_roots(g);
 vector<u32> r(rs.size());
 for(size_t i= 0; i < rs.size(); ++i) r[i]= rs[i].val();
 return r;
}
