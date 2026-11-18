#pragma once
// 書き比べの共通部分: 速い核の手直しの試し (_shared/sps/ranked_xor_v3.hpp) の egf_proj_n を、lib-neo (sps::power_projection) と同じく run の中で
// 答えの列を取って呼ぶ。旗 (RX3_CMP、RX3_PF) は include する提出が決める。速い核の範囲の外は lib-neo と同じく sps::power_projection。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/power_projection.hpp"
#include "_shared/sps/ranked_xor_v3.hpp"
struct Solver {
 using Z= ZMod<MOD>;
 int M;
 vector<Z> f, x, h;
 Solver(int, int M_, const vector<u32>& a, const vector<u32>& w): M(M_), f(a.size()), x(w.size()) {
  for(size_t i= 0; i < a.size(); ++i) f[i]= Z::raw(a[i]), x[i]= Z::raw(w[i]);
 }
 void run() {
  const int n= __builtin_ctzll(f.size());
  if(n < 5 || n > sps_xor3::NMAX) {
   h= sps::power_projection(f, x, M);
   return;
  }
  const sps_xor3::MK k(MOD);
  vector<u32> v;
  sps_xor3::dispatch<5>(n, [&]<int N>() { v= sps_xor3::egf_proj_n<N>(sps_xor3::raw(f), sps_xor3::raw(x), k); });
  h.assign(M, Z());
  sps_xor3::proj_post(v, f[0].val(), M, sps_xor3::raw(h), k);
 }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
