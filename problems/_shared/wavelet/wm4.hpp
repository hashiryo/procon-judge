#pragma once
// 1 段で 2 bit ずつ分ける 4 分の wavelet matrix。wm.hpp の 2 分のものと比べるために、yosupo-range-kth-smallest の wm4
// が使う。値を 0 以上 sigma 未満の順位に詰め、上の 2 bit から段ごとに、その 2 bit (0 から 3 の記号) の順に安定に分ける。
// 段の数は 2 分の半分 (切り上げ) で、kth で待つ読み出しの段も半分になる。段ごとに 64 個ずつのブロックを、記号の上の
// bit の列 h、下の bit の列 l と、ブロックより前の記号 0、1、2 の数 (32 bit ずつ) の 32 byte に詰める。区間の記号ごとの
// 数は、両端のブロックを 1 つずつ読み、popcount を 3 回ずつで求める。k 番目は、記号 0、0 から 1、0 から 2 の数と k を
// 比べた 3 つのマスク (順に包含) で記号を選ぶ (分岐なし)。設計は algo-notes の notes/wavelet-matrix.md。
#include <array>
#include "wm.hpp"
namespace wavelet {
struct WM4 {
 struct alignas(32) Blk {
  u64 h, l;
  u32 c[3], pad;
 };
 int n= 0, levels= 1, nb= 0;
 u32 sigma= 0;
 Vec<Blk> b;                         // 段 d のブロック j は b[d * nb + j]。段 d は下から d 番目の 2 bit
 std::vector<std::array<int, 4>> z;  // z[d][s] は段 d の記号 s より小さい記号の数
 // v は 0 以上 sigma 未満。
 WM4(const std::vector<u32>& v, u32 sigma_): n(v.size()), sigma(sigma_) {
  int lg= 1;
  while((u32(1) << lg) < sigma) ++lg;
  levels= (lg + 1) / 2, nb= n / 64 + 1;
  b.assign(std::size_t(nb) * levels, Blk{});
  z.assign(levels, {});
  std::vector<u32> cur(v), nxt(n);
  for(int d= levels - 1; d >= 0; --d) {
   const int sh= 2 * d;
   Blk* B= b.data() + std::size_t(d) * nb;
   int cnt[4]= {};
   for(int j= 0; j < nb; ++j) {
    B[j].c[0]= cnt[0], B[j].c[1]= cnt[1], B[j].c[2]= cnt[2];
    const int m= std::max(0, std::min(64, n - j * 64));
    u64 hi= 0, lo= 0;
    for(int t= 0; t < m; ++t) {
     const u32 sy= cur[j * 64 + t] >> sh & 3;
     hi|= u64(sy >> 1) << t, lo|= u64(sy & 1) << t, ++cnt[sy];
    }
    B[j].h= hi, B[j].l= lo;
   }
   z[d]= {0, cnt[0], cnt[0] + cnt[1], cnt[0] + cnt[1] + cnt[2]};
   int pos[4]= {z[d][0], z[d][1], z[d][2], z[d][3]};
   for(int i= 0; i < n; ++i) nxt[pos[cur[i] >> sh & 3]++]= cur[i];
   cur.swap(nxt);
  }
 }
 // [l, r) の小さい方から k 番目の値の順位。0 <= k < r - l。
 u32 kth(int l, int r, int k) const {
  u32 res= 0;
  for(int d= levels - 1; d >= 0; --d) {
   const Blk *B= b.data() + std::size_t(d) * nb, &bl= B[l >> 6], &br= B[r >> 6];
   const u64 ml= (u64(1) << (l & 63)) - 1, mr= (u64(1) << (r & 63)) - 1;
   const int l0= bl.c[0] + __builtin_popcountll(~bl.h & ~bl.l & ml), l1= bl.c[1] + __builtin_popcountll(~bl.h & bl.l & ml);
   const int l2= bl.c[2] + __builtin_popcountll(bl.h & ~bl.l & ml), l3= l - l0 - l1 - l2;
   const int r0= br.c[0] + __builtin_popcountll(~br.h & ~br.l & mr), r1= br.c[1] + __builtin_popcountll(~br.h & br.l & mr);
   const int r2= br.c[2] + __builtin_popcountll(br.h & ~br.l & mr), r3= r - r0 - r1 - r2;
   const int a0= r0 - l0, a01= a0 + (r1 - l1), a012= a01 + (r2 - l2);
   const int g1= mask(k >= a0), g2= mask(k >= a01), g3= mask(k >= a012);
   const std::array<int, 4>& Z= z[d];
   k-= sel(g3, a012, sel(g2, a01, a0 & g1));
   l= sel(g3, Z[3] + l3, sel(g2, Z[2] + l2, sel(g1, Z[1] + l1, l0)));
   r= sel(g3, Z[3] + r3, sel(g2, Z[2] + r2, sel(g1, Z[1] + r1, r0)));
   res= res << 2 | u32((g1 & 1) + (g2 & 1) + (g3 & 1));
  }
  return res;
 }
};
}
