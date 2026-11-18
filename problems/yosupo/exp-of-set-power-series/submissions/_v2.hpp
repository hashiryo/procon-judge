#pragma once
// 書き比べの共通部分: 速い核の手直しの試し (_shared/sps/ranked_xor_v2.hpp) の exp_n を、lib-neo と同じく run の中で答えの列を
// 取って呼ぶ。旗 (RX2_R4、RX2_DOT2) と、上の段を列ごとに済ませる形 (ranked_xor_col.hpp) を使うかどうか (RX2_COL) は
// include する提出が決める。n < 5 は lib-neo と同じく sps::exp。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/exp.hpp"
#include "_shared/sps/ranked_xor_col.hpp"
#ifdef RX2_COL
#define RX2_FN sps_xor2::exp_col_n
#else
#define RX2_FN sps_xor2::exp_n
#endif
struct Solver {
 using Z= ZMod<MOD>;
 int n;
 vector<Z> f, h;
 Solver(int n_, const vector<u32>& b): n(n_), f(b.size()) {
  for(size_t i= 0; i < b.size(); ++i) f[i]= Z::raw(b[i]);
 }
 void run() {
  vector<Z> r(f.size());
  const sps_xor2::MK k(MOD);
  if(sps_xor2::dispatch<5>(n, [&]<int N>() { RX2_FN<N>(sps_xor2::raw(f), sps_xor2::raw(r), k); })) h= std::move(r);
  else h= sps::exp(f);
 }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
