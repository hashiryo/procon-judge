#pragma once
// 書き比べの共通部分: 速い核の手直しの試し (_shared/sps/ranked_xor_v3.hpp) の egf_comp_n を、lib-neo (sps::poly_comp) と同じく run の中で
// 答えの列を取って呼ぶ。旗 (RX3_CMP、RX3_PF) は include する提出が決める。速い核の範囲の外は lib-neo と同じく sps::poly_comp。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/composite.hpp"
#include "_shared/sps/ranked_xor_v3.hpp"
struct Solver {
 using Z= ZMod<MOD>;
 vector<Z> P, g, h;
 Solver(int, int, const vector<u32>& f, const vector<u32>& b): P(f.size()), g(b.size()) {
  for(size_t i= 0; i < f.size(); ++i) P[i]= Z::raw(f[i]);
  for(size_t i= 0; i < b.size(); ++i) g[i]= Z::raw(b[i]);
 }
 void run() {
  const int n= __builtin_ctzll(g.size());
  if(P.empty() || n < 5 || n > sps_xor3::NMAX) {
   h= sps::poly_comp(P, g);
   return;
  }
  vector<Z> r(g.size());
  const sps_xor3::MK k(MOD);
  const vector<u32> e= sps_xor3::taylor(sps_xor3::raw(P), int(P.size()), g[0].val(), n, k);
  sps_xor3::dispatch<5>(n, [&]<int N>() { sps_xor3::egf_comp_n<N>(sps_xor3::raw(g), e.data(), sps_xor3::raw(r), k); });
  h= std::move(r);
 }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
