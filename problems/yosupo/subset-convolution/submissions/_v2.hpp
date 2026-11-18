#pragma once
// 書き比べの共通部分: 速い核の手直しの試し (_shared/sps/ranked_xor_v2.hpp) の convolve_n を、lib-neo と同じく run の中で答えの列を
// 取って呼ぶ。旗 (RX2_R4、RX2_DOT2) は include する提出が決める。n < 4 は lib-neo と同じく sps::convolve。比べ終えたら消す。
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/convolve.hpp"
#include "_shared/sps/ranked_xor_v2.hpp"
struct Solver {
 using Z= ZMod<998244353>;
 int n;
 vector<Z> a, b, c;
 Solver(int n_, const vector<i64>& a_, const vector<i64>& b_): n(n_), a(a_.size()), b(b_.size()) {
  for(size_t i= 0; i < a_.size(); ++i) a[i]= Z::raw(u32(a_[i])), b[i]= Z::raw(u32(b_[i]));
 }
 void run() {
  vector<Z> h(a.size());
  const sps_xor2::MK k(Z::mod());
  if(sps_xor2::dispatch<4>(n, [&]<int N>() { sps_xor2::convolve_n<N>(sps_xor2::raw(a), sps_xor2::raw(b), sps_xor2::raw(h), k); })) c= std::move(h);
  else c= sps::convolve(a, b);
 }
 vector<i64> answer() const {
  vector<i64> r(c.size());
  for(size_t i= 0; i < c.size(); ++i) r[i]= c[i].val();
  return r;
 }
};
