#pragma once
// 実験中の複素数の FFT の核 (B bit ずつ P 桁の分割、誤差に応じて 15 bit と 10 bit を選ぶ) を実行時の法 (ZMod<0>) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/cfft.hpp"
struct Kernel {
 using Z= ZMod<0>;
 static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c, u32 mod) {
  Z::set_mod(mod);
  static std::vector<Z> pa, pb, pc;
  pa.resize(n);
  for (int i= 0; i < n; ++i) pa[i]= Z::raw(a[i]);
  pc.resize(hi - lo);
  if (a == b && n == m) {
   Cfft::convolve_range(pa.data(), n, pa.data(), m, lo, hi, pc.data());
  } else {
   pb.resize(m);
   for (int i= 0; i < m; ++i) pb[i]= Z::raw(b[i]);
   Cfft::convolve_range(pa.data(), n, pb.data(), m, lo, hi, pc.data());
  }
  for (int k= lo; k < hi; ++k) c[k - lo]= pc[k - lo].val();
 }
};
