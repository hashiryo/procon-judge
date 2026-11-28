#pragma once
// NeoLibrary の hurwitz::convolve (二項畳み込み、neo/base/core/poly/hurwitz.hpp) を、既定の核で回す。
// crt.hpp を include しているので、ふつうの畳み込みの既定は ntt_friendly な素数では Ntt (単独の NTT)、
// それ以外は Crt (多素数の CRT) になる。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "neo/base/core/convolve/crt.hpp"
#include "neo/base/core/poly/hurwitz.hpp"

struct Kernel {
 using Z= ZMod<998244353>;
 static void convolve(const u32* a, int n, const u32* b, int m, u32* c) {
  static std::vector<Z> va, vb;
  va.resize(n);
  for (int i= 0; i < n; ++i) va[i]= Z::raw(a[i]);
  std::vector<Z> vc;
  if (a == b && n == m) {
   vc= hurwitz::convolve(va, va);
  } else {
   vb.resize(m);
   for (int i= 0; i < m; ++i) vb[i]= Z::raw(b[i]);
   vc= hurwitz::convolve(va, vb);
  }
  for (int k= 0; k < n + m - 1; ++k) c[k]= vc[k].val();
 }
};
