#pragma once
// NeoLibrary の sparse_fps::exp (neo/polynomial/sparse_fps.hpp)。この問題で書き比べた sp1 を、法を定数にしない形で写したもの。
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/sparse_fps.hpp"
inline vector<u32> run(int n, const vector<pair<int, u32>>& f) {
 using Z= ZMod<998244353>;
 vector<Z> a(n);
 for(auto [i, v]: f) a[i]= Z::raw(v);
 const vector<Z> g= sparse_fps::exp(a, n);
 vector<u32> r(g.size());
 for(size_t i= 0; i < g.size(); ++i) r[i]= g[i].val();
 return r;
}
