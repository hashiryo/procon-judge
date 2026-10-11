#pragma once
// 1 段で 2 bit ずつ分ける 4 分の wavelet matrix。wm.hpp の 2 分のものと比べるために、yosupo の 4 問の wm4* が使う。
// 値を 0 以上 sigma 未満の順位に詰め、上の 2 bit から段ごとに、その 2 bit (0 から 3 の記号) の順に安定に分ける。
// 段の数は 2 分の半分 (切り上げ) で、問い合わせで待つ読み出しの段も半分になる。段ごとに 64 個ずつのブロックを、
// 記号の上の bit の列 h、下の bit の列 l と、ブロックより前の記号 0 から 3 の数 (32 bit ずつ) の 32 byte に詰める。
// [0, i) の記号 s の数は、ブロックの累積と、s に合わせて h と l を反転した 2 本の and の popcount で求める。
// k 番目は、記号 0、0 から 1、0 から 2 の数と k を比べた 3 つのマスク (順に包含) で記号を選ぶ (分岐なし)。
// Sum = true なら、段ごとに、その段の並びで記号が 1 未満、2 未満、3 未満のものの重みの累積を 3 本持ち、x 未満の
// 値の重みの和を引ける (x の記号が 0 の段は累積を読まない)。設計は algo-notes の notes/wavelet-matrix.md。
#include <array>
#include "wm.hpp"
namespace wavelet {
template <bool Sum= false> struct WM4 {
 struct alignas(32) Blk {
  u64 h, l;
  u32 c[4];
 };
 int n= 0, levels= 1, nb= 0;
 u32 sigma= 0;
 Vec<Blk> b;                         // 段 d のブロック j は b[d * nb + j]。段 d は下から d 番目の 2 bit
 std::vector<std::array<int, 4>> z;  // z[d][s] は段 d の記号 s より小さい記号の数
 Vec<i64> sums;                      // Sum のとき。段 d の記号 k 未満 (k = 1, 2, 3) の累積は sums[(3d + k - 1)(n + 1) + i]。
                                     // その後ろに元の並びの累積と、最後の段で分けたあとの並びの累積
 // v は 0 以上 sigma 未満。Sum のときは w に重みを渡す。
 WM4(const std::vector<u32>& v, u32 sigma_, const std::vector<i64>* w= nullptr): n(v.size()), sigma(sigma_) {
  int lg= 1;
  while((u32(1) << lg) < sigma) ++lg;
  levels= (lg + 1) / 2, nb= n / 64 + 1;
  b.assign(std::size_t(nb) * levels, Blk{});
  z.assign(levels, {});
  std::vector<u32> cur(v), nxt(n);
  std::vector<i64> cw, nw;
  if constexpr(Sum) {
   cw= *w, nw.resize(n), sums.assign(std::size_t(3 * levels + 2) * (n + 1), 0);
   i64* S= sums.data() + std::size_t(3 * levels) * (n + 1);
   for(int i= 0; i < n; ++i) S[i + 1]= S[i] + cw[i];
  }
  for(int d= levels - 1; d >= 0; --d) {
   const int sh= 2 * d;
   Blk* B= b.data() + std::size_t(d) * nb;
   int cnt[4]= {};
   for(int j= 0; j < nb; ++j) {
    for(int s= 0; s < 4; ++s) B[j].c[s]= cnt[s];
    const int m= std::max(0, std::min(64, n - j * 64));
    u64 hi= 0, lo= 0;
    for(int t= 0; t < m; ++t) {
     const u32 sy= cur[j * 64 + t] >> sh & 3;
     hi|= u64(sy >> 1) << t, lo|= u64(sy & 1) << t, ++cnt[sy];
    }
    B[j].h= hi, B[j].l= lo;
   }
   z[d]= {0, cnt[0], cnt[0] + cnt[1], cnt[0] + cnt[1] + cnt[2]};
   if constexpr(Sum) {
    for(int k= 1; k <= 3; ++k) {
     i64* S= sums.data() + std::size_t(3 * d + k - 1) * (n + 1);
     for(int i= 0; i < n; ++i) S[i + 1]= S[i] + (int(cur[i] >> sh & 3) < k ? cw[i] : 0);
    }
   }
   int pos[4]= {z[d][0], z[d][1], z[d][2], z[d][3]};
   for(int i= 0; i < n; ++i) {
    const int j= pos[cur[i] >> sh & 3]++;
    nxt[j]= cur[i];
    if constexpr(Sum) nw[j]= cw[i];
   }
   cur.swap(nxt);
   if constexpr(Sum) cw.swap(nw);
  }
  if constexpr(Sum) {
   i64* S= sums.data() + std::size_t(3 * levels + 1) * (n + 1);
   for(int i= 0; i < n; ++i) S[i + 1]= S[i] + cw[i];
  }
 }
 // 記号 s の [0, i) の数。hs と ls は、s の上の bit と下の bit が 1 なら全部 1、0 なら 0 のマスク。
 static int rank_eq(const Blk& k, int i, int s, u64 hs, u64 ls) {
  const u64 m= (u64(1) << (i & 63)) - 1;
  return k.c[s] + __builtin_popcountll(~(k.h ^ hs) & ~(k.l ^ ls) & m);
 }
 // 記号 s 未満の [0, i) の数。s = 0 なら 0。記号 s 未満の印は、s = 1 なら h = l = 0、2 なら h = 0、3 なら h と l の
 // 少なくとも一方が 0。マスクで分岐なしに選ぶ。
 static int rank_lt(const Blk& k, int i, int s) {
  const u64 m= (u64(1) << (i & 63)) - 1, m2= -u64(s >= 2), m3= -u64(s == 3), m1= -u64(s == 1);
  const u64 a= m2 | (m1 & ~k.l), b3= m3 & ~k.l;
  const u32 k1= -u32(s >= 1), k2= -u32(s >= 2), k3= -u32(s >= 3);
  return int((k.c[0] & k1) + (k.c[1] & k2) + (k.c[2] & k3)) + __builtin_popcountll(((~k.h & a) | (k.h & b3)) & m);
 }
 // [l, r) の順位 x 未満の値の数。x は 0 以上 sigma 以下。両端とも段ごとに popcount 2 回 (s 未満の数と s の数)。
 int count_less(int l, int r, u32 x) const {
  if(x >= sigma) return r - l;
  int c= 0;
  for(int d= levels - 1; d >= 0; --d) {
   const int s= x >> (2 * d) & 3;
   const u64 hs= -u64(s >> 1), ls= -u64(s & 1);
   const Blk *B= b.data() + std::size_t(d) * nb, &bl= B[l >> 6], &br= B[r >> 6];
   const int zs= z[d][s];
   c+= rank_lt(br, r, s) - rank_lt(bl, l, s);
   l= zs + rank_eq(bl, l, s, hs, ls);
   r= zs + rank_eq(br, r, s, hs, ls);
  }
  return c;
 }
 // [l, r) の小さい方から k 個の重みの和。0 <= k <= r - l。Sum のときだけ。kth と同じく下り、記号 s へ行く段で
 // 記号 s 未満の重みを足す (s = 0 の段は累積を読まない)。最後の段では、残りの k 個は値の同じ並びの先頭 k 個なので、
 // 最後の並びの累積で足す。
 i64 kth_sum(int l, int r, int k) const {
  static_assert(Sum);
  i64 sum= 0;
  for(int d= levels - 1; d >= 0; --d) {
   const Blk *B= b.data() + std::size_t(d) * nb, &bl= B[l >> 6], &br= B[r >> 6];
   const u64 ml= (u64(1) << (l & 63)) - 1, mr= (u64(1) << (r & 63)) - 1;
   const int l0= bl.c[0] + __builtin_popcountll(~bl.h & ~bl.l & ml), l1= bl.c[1] + __builtin_popcountll(~bl.h & bl.l & ml);
   const int l2= bl.c[2] + __builtin_popcountll(bl.h & ~bl.l & ml), l3= l - l0 - l1 - l2;
   const int r0= br.c[0] + __builtin_popcountll(~br.h & ~br.l & mr), r1= br.c[1] + __builtin_popcountll(~br.h & br.l & mr);
   const int r2= br.c[2] + __builtin_popcountll(br.h & ~br.l & mr), r3= r - r0 - r1 - r2;
   const int a0= r0 - l0, a01= a0 + (r1 - l1), a012= a01 + (r2 - l2);
   const int g1= mask(k >= a0), g2= mask(k >= a01), g3= mask(k >= a012), s= (g1 & 1) + (g2 & 1) + (g3 & 1);
   if(s) {
    const i64* S= sums.data() + std::size_t(3 * d + s - 1) * (n + 1);
    sum+= S[r] - S[l];
   }
   const std::array<int, 4>& Z= z[d];
   k-= sel(g3, a012, sel(g2, a01, a0 & g1));
   l= sel(g3, Z[3] + l3, sel(g2, Z[2] + l2, sel(g1, Z[1] + l1, l0)));
   r= sel(g3, Z[3] + r3, sel(g2, Z[2] + r2, sel(g1, Z[1] + r1, r0)));
  }
  const i64* S= sums.data() + std::size_t(3 * levels + 1) * (n + 1);
  return sum + S[l + k] - S[l];
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
 // [l, r) の順位 x の値の数。x < sigma。両端とも段ごとに popcount 1 回。
 int count_eq(int l, int r, u32 x) const {
  for(int d= levels - 1; d >= 0; --d) {
   const int s= x >> (2 * d) & 3;
   const u64 hs= -u64(s >> 1), ls= -u64(s & 1);
   const Blk* B= b.data() + std::size_t(d) * nb;
   const int zs= z[d][s];
   l= zs + rank_eq(B[l >> 6], l, s, hs, ls);
   r= zs + rank_eq(B[r >> 6], r, s, hs, ls);
  }
  return r - l;
 }
 // [l, r) の順位 x 未満の値の数と重みの和。x は 0 以上 sigma 以下。Sum のときだけ。
 std::pair<int, i64> count_sum_less(int l, int r, u32 x) const {
  static_assert(Sum);
  if(x >= sigma) {
   const i64* S= sums.data() + std::size_t(3 * levels) * (n + 1);
   return {r - l, S[r] - S[l]};
  }
  int c= 0;
  i64 sum= 0;
  for(int d= levels - 1; d >= 0; --d) {
   const int s= x >> (2 * d) & 3;
   const u64 hs= -u64(s >> 1), ls= -u64(s & 1);
   const Blk *B= b.data() + std::size_t(d) * nb, &bl= B[l >> 6], &br= B[r >> 6];
   const int zs= z[d][s];
   if(s) {
    // 記号 s 未満の数はブロックから、重みの和は累積の配列から求める。
    const i64* S= sums.data() + std::size_t(3 * d + s - 1) * (n + 1);
    c+= rank_lt(br, r, s) - rank_lt(bl, l, s), sum+= S[r] - S[l];
   }
   l= zs + rank_eq(bl, l, s, hs, ls);
   r= zs + rank_eq(br, r, s, hs, ls);
  }
  return {c, sum};
 }
};
}
