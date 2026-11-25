#pragma once
// 基準の提出: 今の Library の NTT (mylib/fft/NTT.hpp)。dft と idft をそのまま呼ぶ。
// u32 と ModInt の変換は計測区間の中に入る。
#include "mylib/fft/NTT.hpp"
struct Kernel {
 using Z= ModInt<998244353>;
 static void dft(u32* a, int n) {
  static vector<Z> buf;
  if ((int)buf.size() < n) buf.resize(n);
  for (int i= 0; i < n; ++i) buf[i]= Z(a[i]);
  NTT<Z>::dft(n, buf.data());
  for (int i= 0; i < n; ++i) a[i]= buf[i].val();
 }
 static void idft(u32* a, int n) {
  static vector<Z> buf;
  if ((int)buf.size() < n) buf.resize(n);
  for (int i= 0; i < n; ++i) buf[i]= Z(a[i]);
  NTT<Z>::idft(n, buf.data());
  for (int i= 0; i < n; ++i) a[i]= buf[i].val();
 }
};
