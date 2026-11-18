#pragma once
// 書き比べの共通部分: 速い核の手直しの試し (_shared/sps/ranked_xor_v2.hpp) の taylor と egf_comp_n を、lib-neo (sps::poly_comp) と
// 同じ順に run の中で呼ぶ。旗 (RX2_R4、RX2_DOT2) と、上の段を列ごとに済ませる形 (ranked_xor_col.hpp) を使うかどうか
// (RX2_COL) は include する提出が決める。n < 5 は lib-neo と同じく sps::poly_comp。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/composite.hpp"
#include "_shared/sps/ranked_xor_col.hpp"
#ifdef RX2_COL
#define RX2_FN sps_xor2::egf_comp_col_n
#else
#define RX2_FN sps_xor2::egf_comp_n
#endif
struct Solver {
 using Z= ZMod<MOD>;
 vector<Z> P, g, h;
 Solver(int, int, const vector<u32>& f, const vector<u32>& b): P(f.size()), g(b.size()) {
  for(size_t i= 0; i < f.size(); ++i) P[i]= Z::raw(f[i]);
  for(size_t i= 0; i < b.size(); ++i) g[i]= Z::raw(b[i]);
 }
 void run() {
  const int n= __builtin_ctzll(g.size());
  if(P.empty() || n < 5 || n > sps_xor2::NMAX) {
   h= sps::poly_comp(P, g);
   return;
  }
  vector<Z> r(g.size());
  const sps_xor2::MK k(MOD);
  const vector<u32> e= sps_xor2::taylor(sps_xor2::raw(P), int(P.size()), g[0].val(), n, k);
  sps_xor2::dispatch<5>(n, [&]<int N>() { RX2_FN<N>(sps_xor2::raw(g), e.data(), sps_xor2::raw(r), k); });
  h= std::move(r);
 }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
