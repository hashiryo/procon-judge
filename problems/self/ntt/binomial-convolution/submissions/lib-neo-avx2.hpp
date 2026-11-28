#pragma once
// NeoLibrary の hurwitz::convolve を、実験中の単独の NTT の核 (3 本目、AVX2) と、それを中で回す
// 多素数の核 (crt_engine<NttAvx2>) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "neo/base/core/convolve/crt.hpp"
#include "neo/base/core/poly/hurwitz.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

struct Kernel {
 using Z= ZMod<998244353>;
 using Crt= crt_engine<NttAvx2>;
 static void convolve(const u32* a, int n, const u32* b, int m, u32* c) {
  static std::vector<Z> va, vb;
  va.resize(n);
  for (int i= 0; i < n; ++i) va[i]= Z::raw(a[i]);
  std::vector<Z> vc;
  if (a == b && n == m) {
   vc= hurwitz::convolve<NttAvx2, Crt>(va, va);
  } else {
   vb.resize(m);
   for (int i= 0; i < m; ++i) vb[i]= Z::raw(b[i]);
   vc= hurwitz::convolve<NttAvx2, Crt>(va, vb);
  }
  for (int k= 0; k < n + m - 1; ++k) c[k]= vc[k].val();
 }
};
