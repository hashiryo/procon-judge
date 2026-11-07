#pragma once
// 1 段で 3 bit ずつ分ける 8 分の wavelet matrix。wm4.hpp の 4 分の形より段の数を 2/3 にして、問い合わせで待つ読み出しの段を
// 減らせるかを試す。値を 0 以上 sigma 未満の順位に詰め、上の 3 bit から段ごとに、その 3 bit (0 から 7 の記号) の順に安定に分ける。
// 段ごとに 64 個ずつのブロックを、記号の 3 本の bit の列 (b2 が上の bit) と、ブロックより前の記号 t 未満の数 c[t] (t = 0..8) の
// 64 byte (1 本のキャッシュライン) に詰める。[0, i) の記号 t 未満の数は、c[t] と、t 未満の印の列の popcount で求める。
// k 番目は、両端で記号 1 から 7 未満の数を 7 個ずつ求め、区間の中の数と k を比べた 7 つの比較の和で記号を選ぶ (分岐なし)。
// 個数は、x の記号が段ごとに決まっているので、印の列の作り方を先に決めておける。設計は algo-notes の notes/wavelet-matrix.md。
#include <array>
#include "wm.hpp"
namespace wavelet {
struct WM8 {
 struct alignas(64) Blk {
  u64 b0, b1, b2;
  u32 c[9];
 };
 int n= 0, levels= 1, nb= 0;
 u32 sigma= 0;
 Vec<Blk> b;                         // 段 d のブロック j は b[d * nb + j]。段 d は下から d 番目の 3 bit
 std::vector<std::array<int, 8>> z;  // z[d][s] は段 d の記号 s より小さい記号の数
 // v は 0 以上 sigma 未満。
 WM8(const std::vector<u32>& v, u32 sigma_): n(v.size()), sigma(sigma_) {
  int lg= 1;
  while((u32(1) << lg) < sigma) ++lg;
  levels= (lg + 2) / 3, nb= n / 64 + 1;
  b.assign(std::size_t(nb) * levels, Blk{});
  z.assign(levels, {});
  std::vector<u32> cur(v), nxt(n);
  for(int d= levels - 1; d >= 0; --d) {
   const int sh= 3 * d;
   Blk* B= b.data() + std::size_t(d) * nb;
   int cnt[8]= {};
   for(int j= 0; j < nb; ++j) {
    for(int t= 0, s= 0; t <= 8; ++t) B[j].c[t]= s, s+= t < 8 ? cnt[t] : 0;
    const int m= std::max(0, std::min(64, n - j * 64));
    u64 x0= 0, x1= 0, x2= 0;
    for(int t= 0; t < m; ++t) {
     const u32 sy= cur[j * 64 + t] >> sh & 7;
     x0|= u64(sy & 1) << t, x1|= u64(sy >> 1 & 1) << t, x2|= u64(sy >> 2) << t, ++cnt[sy];
    }
    B[j].b0= x0, B[j].b1= x1, B[j].b2= x2;
   }
   for(int s= 0, acc= 0; s < 8; ++s) z[d][s]= acc, acc+= cnt[s];
   int pos[8];
   for(int s= 0; s < 8; ++s) pos[s]= z[d][s];
   for(int i= 0; i < n; ++i) nxt[pos[cur[i] >> sh & 7]++]= cur[i];
   cur.swap(nxt);
  }
 }
 // ブロック k の [0, i) のうち記号 t 未満 (t = 1..7) の数を L[t] に、L[0] = 0、L[8] = i を書く。
 [[gnu::always_inline]] static void lt_all(const Blk& k, int i, int* L) {
  const u64 m= (u64(1) << (i & 63)) - 1, n2= ~k.b2, n1= ~k.b1, a10= k.b1 & k.b0, o10= ~(k.b1 | k.b0);
  L[0]= 0;
  L[1]= k.c[1] + __builtin_popcountll(n2 & o10 & m);
  L[2]= k.c[2] + __builtin_popcountll(n2 & n1 & m);
  L[3]= k.c[3] + __builtin_popcountll(n2 & ~a10 & m);
  L[4]= k.c[4] + __builtin_popcountll(n2 & m);
  L[5]= k.c[5] + __builtin_popcountll((n2 | o10) & m);
  L[6]= k.c[6] + __builtin_popcountll((n2 | n1) & m);
  L[7]= k.c[7] + __builtin_popcountll(~(k.b2 & a10) & m);
  L[8]= i;
 }
 // lt_all と同じ値を書く。数を 1 つずつ空の asm に通して、clang が popcount をベクタの表引きに、ブロックの累積との足し算を 4 個
 // ずつのベクタにまとめないようにする。
 [[gnu::always_inline]] static int pc(u32 c, u64 x) {
  int v= int(c) + __builtin_popcountll(x);
  asm("" : "+r"(v));
  return v;
 }
 [[gnu::always_inline]] static void lt_all_s(const Blk& k, int i, int* L) {
  const u64 m= (u64(1) << (i & 63)) - 1, n2= ~k.b2, n1= ~k.b1, a10= k.b1 & k.b0, o10= ~(k.b1 | k.b0);
  L[0]= 0;
  L[1]= pc(k.c[1], n2 & o10 & m);
  L[2]= pc(k.c[2], n2 & n1 & m);
  L[3]= pc(k.c[3], n2 & ~a10 & m);
  L[4]= pc(k.c[4], n2 & m);
  L[5]= pc(k.c[5], (n2 | o10) & m);
  L[6]= pc(k.c[6], (n2 | n1) & m);
  L[7]= pc(k.c[7], ~(k.b2 & a10) & m);
  L[8]= i;
 }
 // [l, r) の小さい方から k 番目の値の順位。0 <= k < r - l。
 u32 kth(int l, int r, int k) const {
  u32 res= 0;
  for(int d= levels - 1; d >= 0; --d) {
   const Blk* B= b.data() + std::size_t(d) * nb;
   int Ll[9], Lr[9], D[9];
   lt_all(B[l >> 6], l, Ll), lt_all(B[r >> 6], r, Lr);
   int sy= 0;
   for(int t= 0; t <= 8; ++t) D[t]= Lr[t] - Ll[t];
   for(int t= 1; t < 8; ++t) sy+= D[t] <= k;
   const std::array<int, 8>& Z= z[d];
   k-= D[sy];
   l= Z[sy] + Ll[sy + 1] - Ll[sy];
   r= Z[sy] + Lr[sy + 1] - Lr[sy];
   res= res << 3 | u32(sy);
  }
  return res;
 }
 // kth と同じ値を返す。kth は区間の中の数 D[t] を配列に書いてから読み直し、gcc は 8 個の 32 bit の書き込みを 1 回のベクタで読む
 // (書き込みからの受け渡しが効かない) ので、ここでは D[t] を配列に書かずに比べ、選んだ記号の数だけを配列から引く。popcount と
 // D[t] は空の asm に通す (通さないと clang が popcount をベクタの表引きに、D[t] の比較をベクタにまとめ、x64-clang で wm4 より
 // 2 割遅かった)。
 u32 kthp(int l, int r, int k) const {
  u32 res= 0;
  for(int d= levels - 1; d >= 0; --d) {
   const Blk* B= b_(d);
   int Ll[9], Lr[9];
   lt_all_s(B[l >> 6], l, Ll), lt_all_s(B[r >> 6], r, Lr);
   int d1= Lr[1] - Ll[1], d2= Lr[2] - Ll[2], d3= Lr[3] - Ll[3], d4= Lr[4] - Ll[4], d5= Lr[5] - Ll[5], d6= Lr[6] - Ll[6], d7= Lr[7] - Ll[7];
   asm("" : "+r"(d1), "+r"(d2), "+r"(d3), "+r"(d4), "+r"(d5), "+r"(d6), "+r"(d7));
   const int sy= int(d1 <= k) + int(d2 <= k) + int(d3 <= k) + int(d4 <= k) + int(d5 <= k) + int(d6 <= k) + int(d7 <= k);
   const std::array<int, 8>& Z= z[d];
   k-= Lr[sy] - Ll[sy];
   l= Z[sy] + Ll[sy + 1] - Ll[sy];
   r= Z[sy] + Lr[sy + 1] - Lr[sy];
   res= res << 3 | u32(sy);
  }
  return res;
 }
 // kth と同じ値を返す。段の中の 3 bit を上から 1 bit ずつ選ぶ (段の中で 2 分の wavelet matrix を 3 段下りるのと同じ)。bit ごとに、
 // 両端でそれまでに選んだ bit の並びに続けて 0 のものの数を popcount 1 回ずつで求めて k と比べ、選んだ側の数 (Sl、Sr) を引き継ぐ。
 // 段ごとの popcount は 6 回で、スタックの配列を使わない。
 u32 kth3(int l, int r, int k) const {
  u32 res= 0;
  for(int d= levels - 1; d >= 0; --d) {
   const Blk *B= b_(d), &bl= B[l >> 6], &br= B[r >> 6];
   const u64 ml= (u64(1) << (l & 63)) - 1, mr= (u64(1) << (r & 63)) - 1;
   int Ll= int(bl.c[4]) + __builtin_popcountll(~bl.b2 & ml), Lr= int(br.c[4]) + __builtin_popcountll(~br.b2 & mr), cnt= Lr - Ll;
   const int g2= mask(k >= cnt);
   k-= cnt & g2;
   int Sl= sel(g2, l - Ll, Ll), Sr= sel(g2, r - Lr, Lr), p= 4 & g2;
   const u64 h2= u64(i64(g2));
   u64 Ml= ~(bl.b2 ^ h2), Mr= ~(br.b2 ^ h2);
   Ll= int(bl.c[p + 2] - bl.c[p]) + __builtin_popcountll(Ml & ~bl.b1 & ml), Lr= int(br.c[p + 2] - br.c[p]) + __builtin_popcountll(Mr & ~br.b1 & mr);
   cnt= Lr - Ll;
   const int g1= mask(k >= cnt);
   k-= cnt & g1;
   Sl= sel(g1, Sl - Ll, Ll), Sr= sel(g1, Sr - Lr, Lr), p+= 2 & g1;
   const u64 h1= u64(i64(g1));
   Ml&= ~(bl.b1 ^ h1), Mr&= ~(br.b1 ^ h1);
   Ll= int(bl.c[p + 1] - bl.c[p]) + __builtin_popcountll(Ml & ~bl.b0 & ml), Lr= int(br.c[p + 1] - br.c[p]) + __builtin_popcountll(Mr & ~br.b0 & mr);
   cnt= Lr - Ll;
   const int g0= mask(k >= cnt);
   k-= cnt & g0;
   Sl= sel(g0, Sl - Ll, Ll), Sr= sel(g0, Sr - Lr, Lr), p+= 1 & g0;
   l= z[d][p] + Sl, r= z[d][p] + Sr;
   res= res << 3 | u32(p);
  }
  return res;
 }
 // [l, r) の順位 x 未満の値の数。x は 0 以上 sigma 以下。両端とも段ごとに popcount 2 回 (s 未満の数と s の数)。
 int count_less(int l, int r, u32 x) const {
  if(x >= sigma) return r - l;
  int c= 0;
  for(int d= levels - 1; d >= 0; --d) {
   const int s= x >> (3 * d) & 7;
   const u64 a2= -u64(s >> 2), a1= -u64(s >> 1 & 1), a0= -u64(s & 1);
   const Blk *B= b.data() + std::size_t(d) * nb, &bl= B[l >> 6], &br= B[r >> 6];
   const u64 ml= (u64(1) << (l & 63)) - 1, mr= (u64(1) << (r & 63)) - 1;
   const auto lt= [&](const Blk& k) { return (~k.b2 & a2) | (~(k.b2 ^ a2) & ((~k.b1 & a1) | (~(k.b1 ^ a1) & ~k.b0 & a0))); };
   const auto eq= [&](const Blk& k) { return ~(k.b2 ^ a2) & ~(k.b1 ^ a1) & ~(k.b0 ^ a0); };
   const int zs= z[d][s];
   c+= int(br.c[s] - bl.c[s]) + __builtin_popcountll(lt(br) & mr) - __builtin_popcountll(lt(bl) & ml);
   l= zs + int(bl.c[s + 1] - bl.c[s]) + __builtin_popcountll(eq(bl) & ml);
   r= zs + int(br.c[s + 1] - br.c[s]) + __builtin_popcountll(eq(br) & mr);
  }
  return c;
 }
 // [l, r) の順位が [a, b) に入る値の数 (a < b)。a と b の記号が同じ段では、両方の記号未満の数が打ち消し合うので数えずに同じ記号へ
 // 下りる。記号が違う段で、記号 a から b 未満の数を足し、そこから下は b の側 (記号 b 未満を足す) と a の側 (記号 a 未満を引く) を
 // 同じループで下りる。
 int count_range(int l, int r, u32 a, u32 b) const {
  if(b >= sigma) return r - l - count_less(l, r, a);
  const auto lt= [](const Blk& k, u64 a2, u64 a1, u64 a0) { return (~k.b2 & a2) | (~(k.b2 ^ a2) & ((~k.b1 & a1) | (~(k.b1 ^ a1) & ~k.b0 & a0))); };
  const auto eq= [](const Blk& k, u64 a2, u64 a1, u64 a0) { return ~(k.b2 ^ a2) & ~(k.b1 ^ a1) & ~(k.b0 ^ a0); };
  int d= levels - 1;
  for(; d >= 0; --d) {
   const int sa= a >> (3 * d) & 7, sb= b >> (3 * d) & 7;
   const Blk *B= b_(d), &bl= B[l >> 6], &br= B[r >> 6];
   const u64 ml= (u64(1) << (l & 63)) - 1, mr= (u64(1) << (r & 63)) - 1;
   const u64 x2= -u64(sb >> 2), x1= -u64(sb >> 1 & 1), x0= -u64(sb & 1);
   if(sa == sb) {
    const int zs= z[d][sb];
    l= zs + int(bl.c[sb + 1] - bl.c[sb]) + __builtin_popcountll(eq(bl, x2, x1, x0) & ml);
    r= zs + int(br.c[sb + 1] - br.c[sb]) + __builtin_popcountll(eq(br, x2, x1, x0) & mr);
    continue;
   }
   const u64 y2= -u64(sa >> 2), y1= -u64(sa >> 1 & 1), y0= -u64(sa & 1);
   int c= int(br.c[sb] - bl.c[sb]) + __builtin_popcountll(lt(br, x2, x1, x0) & mr) - __builtin_popcountll(lt(bl, x2, x1, x0) & ml);
   c-= int(br.c[sa] - bl.c[sa]) + __builtin_popcountll(lt(br, y2, y1, y0) & mr) - __builtin_popcountll(lt(bl, y2, y1, y0) & ml);
   int lb= z[d][sb] + int(bl.c[sb + 1] - bl.c[sb]) + __builtin_popcountll(eq(bl, x2, x1, x0) & ml);
   int rb= z[d][sb] + int(br.c[sb + 1] - br.c[sb]) + __builtin_popcountll(eq(br, x2, x1, x0) & mr);
   int la= z[d][sa] + int(bl.c[sa + 1] - bl.c[sa]) + __builtin_popcountll(eq(bl, y2, y1, y0) & ml);
   int ra= z[d][sa] + int(br.c[sa + 1] - br.c[sa]) + __builtin_popcountll(eq(br, y2, y1, y0) & mr);
   for(int e= d - 1; e >= 0; --e) {
    const Blk* E= b_(e);
    const int tb= b >> (3 * e) & 7, ta= a >> (3 * e) & 7;
    const u64 p2= -u64(tb >> 2), p1= -u64(tb >> 1 & 1), p0= -u64(tb & 1), q2= -u64(ta >> 2), q1= -u64(ta >> 1 & 1), q0= -u64(ta & 1);
    const Blk &kl= E[lb >> 6], &kr= E[rb >> 6], &hl= E[la >> 6], &hr= E[ra >> 6];
    const u64 mlb= (u64(1) << (lb & 63)) - 1, mrb= (u64(1) << (rb & 63)) - 1, mla= (u64(1) << (la & 63)) - 1, mra= (u64(1) << (ra & 63)) - 1;
    c+= int(kr.c[tb] - kl.c[tb]) + __builtin_popcountll(lt(kr, p2, p1, p0) & mrb) - __builtin_popcountll(lt(kl, p2, p1, p0) & mlb);
    c-= int(hr.c[ta] - hl.c[ta]) + __builtin_popcountll(lt(hr, q2, q1, q0) & mra) - __builtin_popcountll(lt(hl, q2, q1, q0) & mla);
    lb= z[e][tb] + int(kl.c[tb + 1] - kl.c[tb]) + __builtin_popcountll(eq(kl, p2, p1, p0) & mlb);
    rb= z[e][tb] + int(kr.c[tb + 1] - kr.c[tb]) + __builtin_popcountll(eq(kr, p2, p1, p0) & mrb);
    la= z[e][ta] + int(hl.c[ta + 1] - hl.c[ta]) + __builtin_popcountll(eq(hl, q2, q1, q0) & mla);
    ra= z[e][ta] + int(hr.c[ta + 1] - hr.c[ta]) + __builtin_popcountll(eq(hr, q2, q1, q0) & mra);
   }
   return c;
  }
  return 0;
 }
 const Blk* b_(int d) const { return b.data() + std::size_t(d) * nb; }
 // [l, r) の順位 x の値の数。x < sigma。
 int count_eq(int l, int r, u32 x) const {
  for(int d= levels - 1; d >= 0; --d) {
   const int s= x >> (3 * d) & 7;
   const u64 a2= -u64(s >> 2), a1= -u64(s >> 1 & 1), a0= -u64(s & 1);
   const Blk *B= b.data() + std::size_t(d) * nb, &bl= B[l >> 6], &br= B[r >> 6];
   const u64 ml= (u64(1) << (l & 63)) - 1, mr= (u64(1) << (r & 63)) - 1;
   const auto eq= [&](const Blk& k) { return ~(k.b2 ^ a2) & ~(k.b1 ^ a1) & ~(k.b0 ^ a0); };
   const int zs= z[d][s];
   l= zs + int(bl.c[s + 1] - bl.c[s]) + __builtin_popcountll(eq(bl) & ml);
   r= zs + int(br.c[s + 1] - br.c[s]) + __builtin_popcountll(eq(br) & mr);
  }
  return r - l;
 }
};
}
