#pragma once
// 切り分け: NeoLibrary の速い核 (neo/set_power_series/ranked_xor.hpp) を、u32 の列のまま呼ぶ。ZMod の列への写しと戻し (4 MB の列 2 本) は計測に入らない。
// n < 5 は lib-neo と同じく sps::log。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/log.hpp"
#include "neo/set_power_series/ranked_xor.hpp"
inline vector<u32> run(int n, const vector<u32>& b) {
 vector<u32> h(b.size());
 const sps_xor::MK k(MOD);
 if(sps_xor::dispatch<5>(n, [&]<int N>() { sps_xor::log_n<N>(b.data(), h.data(), k); })) return h;
 using Z= ZMod<MOD>;
 vector<Z> f(b.size());
 for(size_t i= 0; i < b.size(); ++i) f[i]= Z::raw(b[i]);
 const vector<Z> g= sps::log(f);
 for(size_t i= 0; i < g.size(); ++i) h[i]= g[i].val();
 return h;
}
