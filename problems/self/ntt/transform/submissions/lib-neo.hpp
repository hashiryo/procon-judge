#pragma once
// 実験中の単独の NTT の核 (1 本目、スカラー、基数 2)。procon-judge の _shared の版を問題の口で呼ぶ。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/ntt_scalar_r2.hpp"
struct Kernel {
 using Z= ZMod<998244353>;
 static void dft(u32* a, int n) {
  static std::vector<Z> buf;
  if ((int)buf.size() < n) buf.resize(n);
  for (int i= 0; i < n; ++i) buf[i]= Z::raw(a[i]);
  Ntt::dft<998244353>(buf.data(), n);
  for (int i= 0; i < n; ++i) a[i]= buf[i].val();
 }
 static void idft(u32* a, int n) {
  static std::vector<Z> buf;
  if ((int)buf.size() < n) buf.resize(n);
  for (int i= 0; i < n; ++i) buf[i]= Z::raw(a[i]);
  Ntt::idft<998244353>(buf.data(), n);
  for (int i= 0; i < n; ++i) a[i]= buf[i].val();
 }
};
