#pragma once
// 書き比べの共通部分: 速い核の手直しの試し (_shared/sps/ranked_xor_v2.hpp) の egf_proj_n と proj_post を、lib-neo
// (sps::power_projection) と同じ順に run の中で呼ぶ。旗 (RX2_R4、RX2_DOT2) と、上の段を列ごとに済ませる形 (ranked_xor_col.hpp)
// を使うかどうか (RX2_COL) は include する提出が決める。n < 5 は lib-neo と同じく sps::power_projection。比べ終えたら消す。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/power_projection.hpp"
#include "_shared/sps/ranked_xor_col.hpp"
#ifdef RX2_COL
#define RX2_FN sps_xor2::egf_proj_col_n
#else
#define RX2_FN sps_xor2::egf_proj_n
#endif
struct Solver {
 using Z= ZMod<MOD>;
 int M;
 vector<Z> f, x, h;
 Solver(int, int M_, const vector<u32>& a, const vector<u32>& w): M(M_), f(a.size()), x(w.size()) {
  for(size_t i= 0; i < a.size(); ++i) f[i]= Z::raw(a[i]), x[i]= Z::raw(w[i]);
 }
 void run() {
  const int n= __builtin_ctzll(f.size());
  if(n < 5 || n > sps_xor2::NMAX) {
   h= sps::power_projection(f, x, M);
   return;
  }
  const sps_xor2::MK k(MOD);
  vector<u32> v;
  sps_xor2::dispatch<5>(n, [&]<int N>() { v= RX2_FN<N>(sps_xor2::raw(f), sps_xor2::raw(x), k); });
  h.assign(M, Z());
  sps_xor2::proj_post(v, f[0].val(), M, sps_xor2::raw(h), k);
 }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
