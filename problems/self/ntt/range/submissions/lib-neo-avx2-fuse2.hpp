#pragma once
// 実験中の単独の NTT の核 (9 本目、基数 4 の段を 2 段まとめて回す形)。procon-judge の _shared の版を問題の口で呼ぶ。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/ntt_avx2_fuse.hpp"
struct Kernel {
 using Z= ZMod<998244353>;
 static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c) {
  static std::vector<Z> pa, pb, pc;
  pa.resize(n);
  for (int i= 0; i < n; ++i) pa[i]= Z::raw(a[i]);
  pc.resize(hi - lo);
  if (a == b && n == m) {
   NttAvx2Fuse2::convolve_range(pa.data(), n, pa.data(), n, lo, hi, pc.data());
  } else {
   pb.resize(m);
   for (int i= 0; i < m; ++i) pb[i]= Z::raw(b[i]);
   NttAvx2Fuse2::convolve_range(pa.data(), n, pb.data(), m, lo, hi, pc.data());
  }
  for (int k= 0; k < hi - lo; ++k) c[k]= pc[k].val();
 }
};
