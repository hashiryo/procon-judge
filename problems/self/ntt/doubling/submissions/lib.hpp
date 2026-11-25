#pragma once
// 基準の提出: 今の Library の NTT (mylib/fft/NTT.hpp)。dft、dft_doubling、idft をそのまま呼ぶ。
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
 static void dft_doubling(u32* a, int n) {
  static vector<Z> buf;
  if ((int)buf.size() < 2 * n) buf.resize(2 * n);
  for (int i= 0; i < n; ++i) buf[i]= Z(a[i]);
  NTT<Z>::dft_doubling(n, buf.data());
  for (int i= 0; i < 2 * n; ++i) a[i]= buf[i].val();
 }
};
