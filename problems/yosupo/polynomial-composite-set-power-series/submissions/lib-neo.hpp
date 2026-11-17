#pragma once
// NeoLibrary の sps::poly_comp (neo/set_power_series/composite.hpp)。xc1 の法を実行時の奇数に広げたもの。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/composite.hpp"
inline vector<u32> run(int, int, const vector<u32>& f, const vector<u32>& b) {
 using Z= ZMod<MOD>;
 vector<Z> P(f.size()), g(b.size());
 for(size_t i= 0; i < f.size(); ++i) P[i]= Z::raw(f[i]);
 for(size_t i= 0; i < b.size(); ++i) g[i]= Z::raw(b[i]);
 const vector<Z> h= sps::poly_comp(P, g);
 vector<u32> r(h.size());
 for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
 return r;
}
