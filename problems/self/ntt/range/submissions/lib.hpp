#pragma once
// 基準の提出: 今の Library の convolve (mylib/fft/convolve.hpp)。全体の積を求めて [lo, hi) を切り出す (範囲だけを求める最適化はしない)。
// a = b のときは同じ配列を 2 回渡し、Library の 2 乗の道に回す。
#include "mylib/fft/convolve.hpp"
struct Kernel {
 using Z= ModInt<998244353>;
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
