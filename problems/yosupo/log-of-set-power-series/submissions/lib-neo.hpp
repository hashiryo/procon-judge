#pragma once
// NeoLibrary の sps::log (neo/set_power_series/log.hpp)。xl1 の法を実行時の奇数に広げたもの。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/log.hpp"
inline vector<u32> run(int, const vector<u32>& b) {
 using Z= ZMod<MOD>;
 vector<Z> f(b.size());
 for(size_t i= 0; i < b.size(); ++i) f[i]= Z::raw(b[i]);
 const vector<Z> h= sps::log(f);
 vector<u32> r(h.size());
 for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
 return r;
}
