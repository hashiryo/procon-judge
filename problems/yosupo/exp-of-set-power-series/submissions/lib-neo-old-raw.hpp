#pragma once
// 切り分け: 直す前の速い核の写し (_shared/sps/ranked_xor_old.hpp、NeoLibrary 3141aee) を、u32 の列のまま呼ぶ。ZMod の列への写しと戻し (4 MB の列 2 本) は計測に入らない。
// n < 5 は lib-neo と同じく sps::exp。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/exp.hpp"
#include "_shared/sps/ranked_xor_old.hpp"
inline vector<u32> run(int n, const vector<u32>& b) {
 vector<u32> h(b.size());
 const sps_xor_old::MK k(MOD);
 if(sps_xor_old::dispatch<5>(n, [&]<int N>() { sps_xor_old::exp_n<N>(b.data(), h.data(), k); })) return h;
 using Z= ZMod<MOD>;
 vector<Z> f(b.size());
 for(size_t i= 0; i < b.size(); ++i) f[i]= Z::raw(b[i]);
 const vector<Z> g= sps::exp(f);
 for(size_t i= 0; i < g.size(); ++i) h[i]= g[i].val();
 return h;
}
