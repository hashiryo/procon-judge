#pragma once
// 基準の提出: 今の Library の convolve (mod 10^9+7 は NTT 素数でないので中で分割する)。
// a = b のときは同じ配列を 2 回渡し、Library の 2 乗の道に回す。
#include "mylib/fft/convolve.hpp"
struct Kernel {
 using Z= ModInt<1000000007>;
 static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c) {
  static vector<Z> pa, pb;
  pa.resize(n);
  for (int i= 0; i < n; ++i) pa[i]= Z(a[i]);
  vector<Z> r;
  if (a == b && n == m) {
   r= convolve<Z>(pa, pa);
  } else {
   pb.resize(m);
   for (int i= 0; i < m; ++i) pb[i]= Z(b[i]);
   r= convolve<Z>(pa, pb);
  }
  for (int k= lo; k < hi; ++k) c[k - lo]= r[k].val();
 }
};
