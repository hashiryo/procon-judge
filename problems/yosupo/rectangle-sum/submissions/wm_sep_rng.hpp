#pragma once
// wm_sep_brtbl の y の範囲の和を、u 未満の和から d 未満の和を引く 2 回の下り方でなく、2 つの境の順位の上の bit が同じ段を 1 回で下り、
// 違う段から下を 2 本同じループで下りる形にしたもの。同じ bit の段は、両方の和が打ち消し合うので累積を読まない。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 wavelet::Table tx, ty;
 explicit Solver(const vector<array<int, 3>>& pts): p(pts), wm(p.yid, p.ys.size(), &p.w), tx(p.xs), ty(p.ys) {}
 const i64* S(int h) const { return wm.sums.data() + size_t(h) * (wm.n + 1); }
 // 位置 [l, r) のうち y の順位が [a, b) に入る点の重みの和 (a < b)。
 i64 sum_range(int l, int r, unsigned a, unsigned b) const {
  if(b >= wm.sigma) return S(wm.lg)[r] - S(wm.lg)[l] - wm.count_sum_less_br(l, r, a).second;
  for(int h= wm.lg - 1; h >= 0; --h) {
   const int l1= wm.bv.rank1(h, l), r1= wm.bv.rank1(h, r);
   if((a ^ b) >> h & 1) {
    const int l0= l - l1, r0= r - r1, z= wm.zero[h];
    i64 s= S(h)[r0] - S(h)[l0];
    int lb= z + l1, rb= z + r1, la= l0, ra= r0;
    for(int g= h - 1; g >= 0; --g) {
     const int p1= wm.bv.rank1(g, lb), q1= wm.bv.rank1(g, rb), p0= lb - p1, q0= rb - q1;
     const int u1= wm.bv.rank1(g, la), v1= wm.bv.rank1(g, ra), u0= la - u1, v0= ra - v1, zg= wm.zero[g];
     if(b >> g & 1) s+= S(g)[q0] - S(g)[p0], lb= zg + p1, rb= zg + q1;
     else lb= p0, rb= q0;
     if(a >> g & 1) s-= S(g)[v0] - S(g)[u0], la= zg + u1, ra= zg + v1;
     else la= u0, ra= v0;
    }
    return s;
   }
   if(b >> h & 1) l= wm.zero[h] + l1, r= wm.zero[h] + r1;
   else l-= l1, r-= r1;
  }
  return 0;
 }
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= tx.lower_cm(l), j= tx.lower_cm(r);
  const unsigned a= ty.lower_cm(d), b= ty.lower_cm(u);
  return a < b ? sum_range(i, j, a, b) : 0;
 }
};
