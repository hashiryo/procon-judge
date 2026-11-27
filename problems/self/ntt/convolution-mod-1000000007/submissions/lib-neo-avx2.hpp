#pragma once
// 実験中の CRT の核 (3 つの NTT の素数 + Garner、中は 3 本目の AVX2) を ZMod<10^9+7> で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/crt_avx2.hpp"
struct Kernel {
 using Z= ZMod<1000000007>;
 static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c) {
  static std::vector<Z> pa, pb, pc;
  pa.resize(n);
  for (int i= 0; i < n; ++i) pa[i]= Z::raw(a[i]);
  pc.resize(hi - lo);
  if (a == b && n == m) {
   CrtAvx2::convolve_range(pa.data(), n, pa.data(), m, lo, hi, pc.data());
  } else {
   pb.resize(m);
   for (int i= 0; i < m; ++i) pb[i]= Z::raw(b[i]);
   CrtAvx2::convolve_range(pa.data(), n, pb.data(), m, lo, hi, pc.data());
  }
  for (int k= lo; k < hi; ++k) c[k - lo]= pc[k - lo].val();
 }
};
