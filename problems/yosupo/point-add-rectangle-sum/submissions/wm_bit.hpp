#pragma once
// 2 分の wavelet matrix の段ごとに BIT を載せる形。最初の点と後から足される点を全部まとめて x の順に並べ、y の順位の列から
// wavelet matrix (problems/_shared/wavelet/wm.hpp の Sep) を作る。段 h の BIT は、段 h で分けたあとの並びの重みを持ち
// (rectangle-sum の wm_sep_br の累積を BIT に替えたもの)、元の並びの重みの BIT も 1 本持つ。足すときは、点の位置を段ごとに rank で
// 追って各段の BIT に足す。和は、y の境の bit が 1 の段だけ、0 の側へ行く範囲を BIT で引く。x の範囲と y の境の順位は、値の範囲に
// 合わせた表 (Lookup) で引く。k 番目に足される点は、構築で受け取った座標の k 番目の点なので、呼ばれた順に数えて位置を引く。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 using u32= unsigned;
 int n= 0, lg= 1, adds= 0;
 u32 sigma= 0;
 vector<int> xs, ys;      // 点の x の昇順と、y の異なる値の昇順
 vector<u32> yid;         // x の順に並べた点の y の順位
 vector<int> spot;        // k 番目に足される点の、x の順の位置
 wavelet::Sep bv;
 vector<int> zero;        // 段 h の 0 の数
 wavelet::Vec<i64> bit;   // 段 h の BIT は bit[h (n + 1) + 1 ..]。段 lg は元の並び
 wavelet::Lookup tx, ty;
 i64* B(int h) { return bit.data() + size_t(h) * (n + 1); }
 const i64* B(int h) const { return bit.data() + size_t(h) * (n + 1); }
 static void make_bit(i64* b, const vector<i64>& w) {
  const int m= w.size();
  for(int i= 1; i <= m; ++i) b[i]= w[i - 1];
  for(int i= 1; i <= m; ++i)
   if(const int j= i + (i & -i); j <= m) b[j]+= b[i];
 }
 // 段 h の並びの先頭 i 個の重みの和。
 i64 pre(int h, int i) const {
  const i64* b= B(h);
  i64 s= 0;
  for(; i > 0; i&= i - 1) s+= b[i];
  return s;
 }
 Solver(const vector<array<i64, 3>>& pts, const vector<array<int, 2>>& spots) {
  const int N= pts.size(), S= spots.size();
  n= N + S;
  vector<int> x(n), y(n);
  vector<i64> w(n, 0);
  for(int i= 0; i < N; ++i) x[i]= int(pts[i][0]), y[i]= int(pts[i][1]), w[i]= pts[i][2];
  for(int k= 0; k < S; ++k) x[N + k]= spots[k][0], y[N + k]= spots[k][1];
  const auto keys= wavelet::sorted_keys(x);
  xs.resize(n);
  vector<int> yo(n), inv(n);
  vector<i64> cw(n);
  for(int j= 0; j < n; ++j) {
   const int i= int(u32(keys[j]));
   xs[j]= x[i], yo[j]= y[i], cw[j]= w[i], inv[i]= j;
  }
  spot.resize(S);
  for(int k= 0; k < S; ++k) spot[k]= inv[N + k];
  auto c= wavelet::compress(yo);
  yid= std::move(c.id), ys= std::move(c.vals), sigma= ys.size();
  while((u32(1) << lg) < sigma) ++lg;
  bv.init(n, lg), zero.assign(lg, 0);
  bit.assign(size_t(lg + 1) * (n + 1), 0);
  make_bit(B(lg), cw);
  vector<u32> cur(yid), nxt(n);
  vector<i64> nw(n);
  for(int h= lg - 1; h >= 0; --h) {
   const int z= n - bv.fill(h, cur.data(), n);
   zero[h]= z;
   int i0= 0, i1= z;
   for(int i= 0; i < n; ++i) {
    const u32 b= cur[i] >> h & 1;
    const int j= b ? i1 : i0;
    nxt[j]= cur[i], nw[j]= cw[i];
    i1+= b, i0+= b ^ 1;
   }
   cur.swap(nxt), cw.swap(nw);
   make_bit(B(h), cw);
  }
  tx= wavelet::Lookup(xs), ty= wavelet::Lookup(ys);
 }
 void add(int, int, i64 w) {
  int i= spot[adds++];
  for(int p= i + 1; p <= n; p+= p & -p) B(lg)[p]+= w;
  const u32 v= yid[i];
  for(int h= lg - 1; h >= 0; --h) {
   const int r1= bv.rank1(h, i);
   i= v >> h & 1 ? zero[h] + r1 : i - r1;
   i64* b= B(h);
   for(int p= i + 1; p <= n; p+= p & -p) b[p]+= w;
  }
 }
 // x の順の位置 [l, r) のうち y の順位が x 未満の点の重みの和。
 i64 sum_less(int l, int r, u32 x) const {
  if(x >= sigma) return pre(lg, r) - pre(lg, l);
  i64 s= 0;
  for(int h= lg - 1; h >= 0; --h) {
   const int l1= bv.rank1(h, l), r1= bv.rank1(h, r), l0= l - l1, r0= r - r1;
   if(x >> h & 1) {
    s+= pre(h, r0) - pre(h, l0);
    l= zero[h] + l1, r= zero[h] + r1;
   } else l= l0, r= r0;
  }
  return s;
 }
 // x の順の位置 [l, r) のうち y の順位が [a, b) に入る点の重みの和 (a < b <= sigma)。a と b の上の bit が同じ段では、両方の和が打ち消し
 // 合うので、BIT を引かずに同じ側へ下りる。違う段からは別々に下りる (b は 1 の側、a は 0 の側)。
 i64 sum_range(int l, int r, u32 a, u32 b) const {
  if(b >= sigma) return pre(lg, r) - pre(lg, l) - sum_less(l, r, a);
  int h= lg - 1;
  for(; h >= 0; --h) {
   const int l1= bv.rank1(h, l), r1= bv.rank1(h, r);
   if((a ^ b) >> h & 1) {
    const int l0= l - l1, r0= r - r1;
    i64 s= pre(h, r0) - pre(h, l0);
    int lb= zero[h] + l1, rb= zero[h] + r1, la= l0, ra= r0;
    for(int g= h - 1; g >= 0; --g) {
     const int p1= bv.rank1(g, lb), q1= bv.rank1(g, rb), p0= lb - p1, q0= rb - q1;
     if(b >> g & 1) s+= pre(g, q0) - pre(g, p0), lb= zero[g] + p1, rb= zero[g] + q1;
     else lb= p0, rb= q0;
     const int u1= bv.rank1(g, la), v1= bv.rank1(g, ra), u0= la - u1, v0= ra - v1;
     if(a >> g & 1) s-= pre(g, v0) - pre(g, u0), la= zero[g] + u1, ra= zero[g] + v1;
     else la= u0, ra= v0;
    }
    return s;
   }
   if(b >> h & 1) l= zero[h] + l1, r= zero[h] + r1;
   else l-= l1, r-= r1;
  }
  return 0;
 }
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= tx.lower(l), j= tx.lower(r);
  const u32 a= ty.lower(d), b= ty.lower(u);
  return a < b ? sum_range(i, j, a, b) : 0;
 }
};
