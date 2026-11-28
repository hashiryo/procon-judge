#pragma once
// NeoLibrary の hurwitz::convolve を、実験中の AVX2 の核で回す。AVX2 の核は NTT に使える素数しか
// 扱えないので、係数の型で振り分ける Avx2Conv (NttAvx2 か crt_engine<NttAvx2>) を渡す。法は実行時 (ZMod<0>)。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "neo/base/core/convolve/crt.hpp"
#include "neo/base/core/poly/hurwitz.hpp"
#include "_shared/ntt/avx2_conv.hpp"

struct Kernel {
 using Z= ZMod<0>;
 using Crt= crt_engine<NttAvx2>;
 static void convolve(const u32* a, int n, const u32* b, int m, u32* c, u32 mod) {
  Z::set_mod(mod);
  static std::vector<Z> va, vb;
  va.resize(n);
  for (int i= 0; i < n; ++i) va[i]= Z::raw(a[i]);
  vb.resize(m);
  for (int i= 0; i < m; ++i) vb[i]= Z::raw(b[i]);
  auto vc= hurwitz::convolve<Avx2Conv, Crt>(va, vb);
  for (int k= 0; k < n + m - 1; ++k) c[k]= vc[k].val();
 }
};
