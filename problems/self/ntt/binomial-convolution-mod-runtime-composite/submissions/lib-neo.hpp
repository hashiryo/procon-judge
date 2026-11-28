#pragma once
// NeoLibrary の hurwitz::convolve (二項畳み込み、neo/base/core/poly/hurwitz.hpp) を、既定の核で回す。
// 法は実行時 (ZMod<0>)。法の素因数が n + m - 1 以上なら階乗で分ける道、そうでなければ
// crt.hpp の Crt (多素数の CRT) で p を抜いた階乗の和を求める道を通る。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "neo/base/core/convolve/crt.hpp"
#include "neo/base/core/poly/hurwitz.hpp"

struct Kernel {
 using Z= ZMod<0>;
 static void convolve(const u32* a, int n, const u32* b, int m, u32* c, u32 mod) {
  Z::set_mod(mod);
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
