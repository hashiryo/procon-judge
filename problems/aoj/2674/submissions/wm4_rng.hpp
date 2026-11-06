#pragma once
// wm4 の [lo, hi) の個数を、2 回下りる形でなく、2 つの境の記号が同じ段を 1 回で下り、違う段から下を 2 本同じループで下りる形に
// したもの。4 分の形の核 (problems/_shared/wavelet/wm4.hpp) はそのままで、ブロックを読む関数をここに置く。
#include "pj.hpp"
#include "_shared/wavelet/wm4.hpp"
struct Solver {
 using WM= wavelet::WM4<>;
 using u64= unsigned long long;
 wavelet::Compressed c;
 WM wm;
 wavelet::Lookup lk;
 explicit Solver(const vector<int>& x): c(wavelet::compress_s(x)), wm(c.id, c.vals.size()), lk(c.vals) {}
 static int eq(const WM::Blk& k, int i, int s) { return WM::rank_eq(k, i, s, -u64(s >> 1), -u64(s & 1)); }
 int count_range(int l, int r, unsigned a, unsigned b) const {
  if(b >= wm.sigma) return r - l - wm.count_less(l, r, a);
  for(int d= wm.levels - 1; d >= 0; --d) {
   const int sa= a >> (2 * d) & 3, sb= b >> (2 * d) & 3;
   const WM::Blk *B= wm.b.data() + size_t(d) * wm.nb, &bl= B[l >> 6], &br= B[r >> 6];
   if(sa == sb) {
    const int zs= wm.z[d][sb];
    l= zs + eq(bl, l, sb), r= zs + eq(br, r, sb);
    continue;
   }
   int cnt= WM::rank_lt(br, r, sb) - WM::rank_lt(bl, l, sb) - (WM::rank_lt(br, r, sa) - WM::rank_lt(bl, l, sa));
   int lb= wm.z[d][sb] + eq(bl, l, sb), rb= wm.z[d][sb] + eq(br, r, sb), la= wm.z[d][sa] + eq(bl, l, sa), ra= wm.z[d][sa] + eq(br, r, sa);
   for(int e= d - 1; e >= 0; --e) {
    const WM::Blk* E= wm.b.data() + size_t(e) * wm.nb;
    const int tb= b >> (2 * e) & 3, ta= a >> (2 * e) & 3;
    const WM::Blk &kl= E[lb >> 6], &kr= E[rb >> 6], &hl= E[la >> 6], &hr= E[ra >> 6];
    cnt+= WM::rank_lt(kr, rb, tb) - WM::rank_lt(kl, lb, tb) - (WM::rank_lt(hr, ra, ta) - WM::rank_lt(hl, la, ta));
    lb= wm.z[e][tb] + eq(kl, lb, tb), rb= wm.z[e][tb] + eq(kr, rb, tb);
    la= wm.z[e][ta] + eq(hl, la, ta), ra= wm.z[e][ta] + eq(hr, ra, ta);
   }
   return cnt;
  }
  return 0;
 }
 int count(int l, int r, int lo, int hi) const {
  const int a= lk.lower(lo), b= lk.lower(hi);
  return a < b ? count_range(l, r, a, b) : 0;
 }
};
