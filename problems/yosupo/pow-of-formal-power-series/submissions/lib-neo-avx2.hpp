#pragma once
// NeoLibrary の fps::pow (exp(k log f)、neo/base/core/poly/fps.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/fps.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> a, b;
 i64 k;

 Solver(const std::vector<i64>& src, i64 m): a(src.size()), k(m) {
  for (size_t i= 0; i < src.size(); ++i) a[i]= Z::raw((unsigned)src[i]);
 }

 void run() {
  const int n= int(a.size());
  if (k >= 0) b= fps::pow<NttAvx2>(a, (unsigned long long)k, n);
  else b= fps::pow<NttAvx2>(fps::inv<NttAvx2>(a, n), ~(unsigned long long)k + 1, n);  // 負の指数は逆数の k 乗
 }

 std::vector<i64> answer() const {
  std::vector<i64> r(b.size());
  for (size_t i= 0; i < b.size(); ++i) r[i]= b[i].val();
  return r;
 }
};
