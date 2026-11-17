#pragma once
// 切り分け: 直す前の速い核の写し (_shared/sps/ranked_xor_old.hpp、NeoLibrary 3141aee) を、u32 の列のまま呼ぶ (egf_proj_n と proj_post)。ZMod の列への写し (4 MB の列 2 本) は計測に入らない。
// n < 5 は lib-neo と同じく sps::power_projection。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/power_projection.hpp"
#include "_shared/sps/ranked_xor_old.hpp"
inline vector<u32> run(int n, int M, const vector<u32>& a, const vector<u32>& w) {
 if(n >= 5 && n <= sps_xor_old::NMAX) {
  const sps_xor_old::MK k(MOD);
  vector<u32> v;
  sps_xor_old::dispatch<5>(n, [&]<int N>() { v= sps_xor_old::egf_proj_n<N>(a.data(), w.data(), k); });
  vector<u32> ans(M);
  sps_xor_old::proj_post(v, a[0], M, ans.data(), k);
  return ans;
 }
 using Z= ZMod<MOD>;
 vector<Z> f(a.size()), x(w.size());
 for(size_t i= 0; i < a.size(); ++i) f[i]= Z::raw(a[i]), x[i]= Z::raw(w[i]);
 const vector<Z> h= sps::power_projection(f, x, M);
 vector<u32> r(h.size());
 for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
 return r;
}
