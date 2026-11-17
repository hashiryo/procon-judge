#pragma once
// NeoLibrary の sps::power_projection (neo/set_power_series/power_projection.hpp)。xp1 の法を実行時の奇数に広げ、
// 答えの段を割り算を使わない漸化式にしたもの。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/power_projection.hpp"
inline vector<u32> run(int, int M, const vector<u32>& a, const vector<u32>& w) {
 using Z= ZMod<MOD>;
 vector<Z> f(a.size()), x(w.size());
 for(size_t i= 0; i < a.size(); ++i) f[i]= Z::raw(a[i]), x[i]= Z::raw(w[i]);
 const vector<Z> h= sps::power_projection(f, x, M);
 vector<u32> r(h.size());
 for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
 return r;
}
