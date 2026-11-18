#pragma once
// 書き比べの共通部分: 速い核の手直しの試し (_shared/sps/ranked_xor_v3.hpp) の exp_n を、lib-neo (sps::exp) と同じく run の中で
// 答えの列を取って呼ぶ。旗 (RX3_CMP、RX3_PF) は include する提出が決める。速い核の範囲の外は lib-neo と同じく sps::exp。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/exp.hpp"
#include "_shared/sps/ranked_xor_v3.hpp"
struct Solver {
 using Z= ZMod<MOD>;
 int n;
 vector<Z> f, h;
 Solver(int n_, const vector<u32>& b): n(n_), f(b.size()) {
  for(size_t i= 0; i < b.size(); ++i) f[i]= Z::raw(b[i]);
 }
 void run() {
  vector<Z> r(f.size());
  const sps_xor3::MK k(MOD);
  if(sps_xor3::dispatch<5>(n, [&]<int N>() { sps_xor3::exp_n<N>(sps_xor3::raw(f), sps_xor3::raw(r), k); })) h= std::move(r);
  else h= sps::exp(f);
 }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
