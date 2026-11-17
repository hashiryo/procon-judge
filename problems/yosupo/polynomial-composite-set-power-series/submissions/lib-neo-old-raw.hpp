#pragma once
// 切り分け: 直す前の速い核の写し (_shared/sps/ranked_xor_old.hpp、NeoLibrary 3141aee) を、u32 の列のまま呼ぶ (taylor と egf_comp_n)。ZMod の列への写しと戻しは計測に入らない。
// n < 5 は lib-neo と同じく sps::poly_comp。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/composite.hpp"
#include "_shared/sps/ranked_xor_old.hpp"
inline vector<u32> run(int M, int n, const vector<u32>& f, const vector<u32>& b) {
 if(n >= 5 && n <= sps_xor_old::NMAX && M > 0) {
  vector<u32> h(b.size());
  const sps_xor_old::MK k(MOD);
  const vector<u32> e= sps_xor_old::taylor(f.data(), M, b[0], n, k);
  sps_xor_old::dispatch<5>(n, [&]<int N>() { sps_xor_old::egf_comp_n<N>(b.data(), e.data(), h.data(), k); });
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
