#pragma once
// 切り分け: 直す前の速い核の写し (_shared/sps/ranked_xor_old.hpp、NeoLibrary 3141aee) の convolve_n を、lib-neo と同じく
// run の中で答えの列を取って呼ぶ。lib-neo との差は核の書き方だけ。n < 4 は lib-neo と同じく sps::convolve。比べ終えたら消す。
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/convolve.hpp"
#include "_shared/sps/ranked_xor_old.hpp"
struct Solver {
 using Z= ZMod<998244353>;
 int n;
 vector<Z> a, b, c;
 Solver(int n_, const vector<i64>& a_, const vector<i64>& b_): n(n_), a(a_.size()), b(b_.size()) {
  for(size_t i= 0; i < a_.size(); ++i) a[i]= Z::raw(u32(a_[i])), b[i]= Z::raw(u32(b_[i]));
 }
 void run() {
  vector<Z> h(a.size());
  const sps_xor_old::MK k(Z::mod());
  if(sps_xor_old::dispatch<4>(n, [&]<int N>() { sps_xor_old::convolve_n<N>(sps_xor_old::raw(a), sps_xor_old::raw(b), sps_xor_old::raw(h), k); })) c= std::move(h);
  else c= sps::convolve(a, b);
 }
 vector<i64> answer() const {
  vector<i64> r(c.size());
  for(size_t i= 0; i < c.size(); ++i) r[i]= c[i].val();
  return r;
 }
};
