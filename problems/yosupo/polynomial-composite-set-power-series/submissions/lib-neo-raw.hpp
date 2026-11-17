#pragma once
// 切り分け: NeoLibrary の速い核 (neo/set_power_series/ranked_xor.hpp) を、u32 の列のまま呼ぶ (taylor と egf_comp_n)。ZMod の列への写しと戻しは計測に入らない。
// n < 5 は lib-neo と同じく sps::poly_comp。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/composite.hpp"
#include "neo/set_power_series/ranked_xor.hpp"
inline vector<u32> run(int M, int n, const vector<u32>& f, const vector<u32>& b) {
 if(n >= 5 && n <= sps_xor::NMAX && M > 0) {
  vector<u32> h(b.size());
  const sps_xor::MK k(MOD);
  const vector<u32> e= sps_xor::taylor(f.data(), M, b[0], n, k);
  sps_xor::dispatch<5>(n, [&]<int N>() { sps_xor::egf_comp_n<N>(b.data(), e.data(), h.data(), k); });
  return h;
 }
 using Z= ZMod<MOD>;
 vector<Z> P(f.size()), g(b.size());
 for(size_t i= 0; i < f.size(); ++i) P[i]= Z::raw(f[i]);
 for(size_t i= 0; i < b.size(); ++i) g[i]= Z::raw(b[i]);
 const vector<Z> h= sps::poly_comp(P, g);
 vector<u32> r(h.size());
 for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
 return r;
}
