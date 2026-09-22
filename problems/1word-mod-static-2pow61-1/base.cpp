// harness: 各 submissions/*.hpp が定義する struct DIV を使って計測する。
// 旧 judge の 1word-mod/static/1word-2^61-1 から移した。
// DIV は { DIV(); u64 mod(u64 a) const; } という interface を持つことを期待する。
// 法はこの base.cpp の MOD で決まる。
#include "pj.hpp"
#include "../_shared/modulo-test/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive64.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 u64 n_, sa_, ma_, mb_;
 cin >> n_ >> sa_ >> ma_ >> mb_;

 constexpr u64 MASK= (u64(1) << 63) - 1;
 constexpr u64 MOD= (1ULL << 61) - 1;

 // 計測前に a クエリ列を全て生成しておく (生成コストを algo time から除外する)。
 vector<u64> as(n_);
 {
  u64 sa= sa_;
  for(u64 i= 0; i < n_; ++i) {
   sa= sa * ma_ + mb_;
   as[i]= sa & MASK;
  }
 }

 // REPEAT は wall time を REPEAT 倍するので TLE と相談して決める。
 constexpr int REPEAT= 1;
 uint64_t best_ns= ~uint64_t(0);
 u64 result_out= 0;

 for(int rep= 0; rep < REPEAT; ++rep) {
  const DIV div(MOD);  // 前計算は計測外。
  u64 acc= 0;
  auto t0= chrono::steady_clock::now();
  for(u64 i= 0; i < n_; ++i) {
   acc^= div.mod(as[i]);
  }
  auto t1= chrono::steady_clock::now();
  result_out= acc;
  auto ns= (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
  if(ns < best_ns) best_ns= ns;
 }

 cout << result_out << '\n';
 report_metrics((long long)best_ns);
 return 0;
}
