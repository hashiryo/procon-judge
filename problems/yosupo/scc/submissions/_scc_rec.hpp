#pragma once
// 頂点ごとの記録に、方式の鍵 (Pearce の rix、path-based の I) と最初の隣接を並べて置く形。_scc.hpp の形では、頂点 w に
// 下りると、鍵と隣接の始まりを読み、その値で隣接の先頭を読んでから次の頂点に進むので、下りる鎖の 1 歩に読み込みが 2 段
// つながる。最初の隣接を鍵と同じ記録に置けば、1 歩が読み込み 1 段で済む。残りの隣接は、R = RecCSR なら CSR (最初の隣接も
// 並んでいて、そこは飛ばす)、R = RecList なら記録に先頭を持つ辺の連結リストから読み、読み始めるのは最初の隣接から帰ったあと。
//
// TRIM が 1 か 2 なら、DFS の前に入次数 0 の頂点を待ち行列で剥がす (Kahn。2 は先読みを入れた peel_pf で、R = RecCSR
// のみ)。剥がした頂点はどれも 1 頂点の成分で、剥がした順がトポロジカル順の先頭に来る。剥がされない頂点から剥がした頂点
// への辺は無いので、DFS は剥がした頂点に触れない。DFS は読み込みが 1 本の鎖につながるが、剥がしは待ち行列の頂点ごとに
// 独立なので、読み込みを並べて出せる。剥がした頂点の鍵は NIL - (剥がした順) にして、DFS の根の走査で飛ばす。
//
// 成分の番号の振り方は _scc.hpp の同じ名前の方式と同じで、TRIM なら剥がした頂点の分だけ後ろへずらす。
#include "_scc.hpp"
namespace scc {
// 記録は鍵と最初の隣接の 8 byte。残りは CSR。CSR を組んでから、頂点の順に 1 回なめて記録を作る。
struct RecCSR {
 struct Rec {
  u32 key, first;
 };
 Rec* rec;
 u32 *off, *adj;
 struct Cur {
  u32 i, e;
 };
 template <class M> static size_t words(size_t n, size_t m) { return M::words(2 * n) + M::words(n + 1) + M::words(m); }
 template <class M> void build(int n, const Edges& es, M& mem) {
  rec= mem.template take_as<Rec>(n);
  CSR g;
  g.template build<0>(n, es, mem);
  off= g.off, adj= g.adj;
  for(int v= 0; v < n; ++v) rec[v]= {0, off[v] < off[v + 1] ? adj[off[v]] : NIL};
 }
 // 最初の隣接は記録から読むので、残りは off[v] + 1 から。隣接が無ければ空。
 Cur rest(u32 v) const {
  const u32 e= off[v + 1];
  return {min(off[v] + 1, e), e};
 }
 static bool done(const Cur& c) { return c.i == c.e; }
 u32 next(Cur& c) const { return adj[c.i++]; }
};
// 記録は鍵と最初の隣接と残りの連結リストの先頭の 16 byte。辺の列を 1 回前から読むだけで組める。
struct RecList {
 struct Rec {
  u32 key, first, head, pad;
 };
 Rec* rec;
 u32* nx;
 struct Cur {
  u32 e;
 };
 template <class M> static size_t words(size_t n, size_t m) { return M::words(4 * n) + M::words(2 * m); }
 template <class M> void build(int n, const Edges& es, M& mem) {
  rec= mem.template take_as<Rec>(n), nx= mem.take(2 * es.size());
  for(int v= 0; v < n; ++v) rec[v]= {0, NIL, NIL, 0};
  for(size_t i= 0; i < es.size(); ++i) {
   Rec& r= rec[es[i][0]];
   const u32 b= es[i][1];
   if(r.first == NIL) r.first= b;
   else nx[2 * i]= b, nx[2 * i + 1]= r.head, r.head= i;
  }
 }
 Cur rest(u32 v) const { return {rec[v].head}; }
 static bool done(const Cur& c) { return c.e == NIL; }
 u32 next(Cur& c) const {
  const u32 w= nx[2 * c.e];
  c.e= nx[2 * c.e + 1];
  return w;
 }
};
// 入次数 0 の頂点を剥がし、剥がした数を返す。剥がした頂点の鍵は NIL - (剥がした順) にする。q と deg は n + 1 語。
// 積むかどうかは分岐にせず、毎回 q の末尾に書いて、入次数が 0 になったときだけ末尾を進める。
template <class R> u32 peel(int n, const Edges& es, R& g, u32* q, u32* deg) {
 memset(deg, 0, n * sizeof(u32));
 for(auto& e: es) ++deg[e[1]];
 u32 qt= 0;
 for(u32 v= 0; v < u32(n); ++v) q[qt]= v, qt+= deg[v] == 0;
 for(u32 qh= 0; qh < qt; ++qh) {
  const u32 v= q[qh];
  auto& r= g.rec[v];
  r.key= NIL - qh;
  if(r.first == NIL) continue;
  u32 w= r.first;
  q[qt]= w, qt+= --deg[w] == 0;
  for(auto c= g.rest(v); !R::done(c);) w= g.next(c), q[qt]= w, qt+= --deg[w] == 0;
 }
 return qt;
}
// peel と同じことを、待ち行列の D 個先の頂点の記録と隣接の始まり、D / 2 個先の頂点の最初の隣接の入次数と残りの隣接を
// 先読みしながら行う。読み込みの結果で決まる分岐は、予想を外すとその読み込みの待ちがそのまま出るので、先読みで待ちを
// 短くする。最初の隣接が無い頂点は、行き先を番兵の n にして同じ手順を踏む (deg[n] は 0 から減らすので 0 に戻らず、積まれない)。
// 待ち行列の末尾より先はまだ決まっていないので、先読みの位置は末尾で止める。
template <u32 D> u32 peel_pf(int n, const Edges& es, RecCSR& g, u32* q, u32* deg) {
 memset(deg, 0, (n + 1) * sizeof(u32));
 for(auto& e: es) ++deg[e[1]];
 u32 qt= 0;
 for(u32 v= 0; v < u32(n); ++v) q[qt]= v, qt+= deg[v] == 0;
 auto* rec= g.rec;
 const u32 *off= g.off, *adj= g.adj;
 for(u32 qh= 0; qh < qt; ++qh) {
  const u32 a= q[min(qh + D, qt - 1)];
  __builtin_prefetch(rec + a), __builtin_prefetch(off + a);
  const u32 b= q[min(qh + D / 2, qt - 1)];
  __builtin_prefetch(deg + min(rec[b].first, u32(n))), __builtin_prefetch(adj + off[b]);
  const u32 v= q[qh];
  auto& r= rec[v];
  r.key= NIL - qh;
  u32 w= min(r.first, u32(n));
  q[qt]= w, qt+= --deg[w] == 0;
  for(u32 i= off[v] + 1, e= off[v + 1]; i < e; ++i) w= adj[i], q[qt]= w, qt+= --deg[w] == 0;
 }
 return qt;
}
// Pearce の省メモリ版 (_scc.hpp の Pearce と同じ) を記録の形で回す。nf は、いまの頂点の最初の隣接のうち、まだ見ていないもの。
template <class R, bool HP, int TRIM> struct PearceRec {
 using M= Mem<HP>;
 struct F {
  u32 v, lv, ov;
  typename R::Cur c;
 };
 M mem;
 R g;
 u32 n, K, base, P= 0;
 void run(int n_, const Edges& es) {
  n= n_;
  const size_t m= es.size();
  mem.reserve(R::template words<M>(n, m) + M::words(n) + M::words(n * sizeof(F) / sizeof(u32)) + (TRIM ? 2 * M::words(n + 1) : 0));
  g.build(n, es, mem);
  auto* rec= g.rec;
  u32* st= mem.take(n);
  F* fs= mem.template take_as<F>(n);
  if constexpr(TRIM) {
   u32 *q= mem.take(n + 1), *deg= mem.take(n + 1);
   if constexpr(TRIM == 1) P= peel(n, es, g, q, deg);
   else P= peel_pf<16>(n, es, g, q, deg);
  }
  u32 index= 1, cc= n - 1, top= 0;
  for(u32 r= 0; r < n; ++r) {
   if(rec[r].key) continue;
   u32 v= r, ov= index++, lv= ov, nf= rec[r].first;
   auto c= g.rest(r);
   F* sp= fs;
   rec[v].key= ov;
   for(;;) {
    u32 w;
    if(nf != NIL) w= nf, nf= NIL;
    else if(!R::done(c)) w= g.next(c);
    else {
     u32 x;
     if(lv == ov) {
      --index;
      while(top && ov <= rec[st[top - 1]].key) rec[st[--top]].key= cc, --index;
      x= rec[v].key= cc--;
     } else x= rec[v].key= lv, st[top++]= v;
     if(sp == fs) break;
     --sp, v= sp->v, ov= sp->ov, lv= sp->lv, c= sp->c;
     if(x < lv) lv= x;
     continue;
    }
    const auto rw= rec[w];
    if(!rw.key) *sp++= {v, lv, ov, c}, v= w, ov= lv= index++, rec[v].key= ov, nf= rw.first, c= g.rest(w);
    else if(rw.key < lv) lv= rw.key;
   }
  }
  K= P + (n - 1 - cc), base= cc + 1;
 }
 int comp(int v) const {
  const u32 x= g.rec[v].key;
  return int(x > n ? NIL - x : x - base + P);
 }
};
// path-based (_scc.hpp の Gabow と同じ) を記録の形で回す。
template <class R, bool HP, int TRIM> struct GabowRec {
 using M= Mem<HP>;
 struct F {
  u32 v;
  typename R::Cur c;
 };
 M mem;
 R g;
 u32 n, K, base, P= 0;
 void run(int n_, const Edges& es) {
  n= n_;
  const size_t m= es.size();
  mem.reserve(R::template words<M>(n, m) + 2 * M::words(n) + M::words(n * sizeof(F) / sizeof(u32)) + (TRIM ? 2 * M::words(n + 1) : 0));
  g.build(n, es, mem);
  auto* rec= g.rec;
  u32 *st= mem.take(n), *bs= mem.take(n);
  F* fs= mem.template take_as<F>(n);
  if constexpr(TRIM) {
   u32 *q= mem.take(n + 1), *deg= mem.take(n + 1);
   if constexpr(TRIM == 1) P= peel(n, es, g, q, deg);
   else P= peel_pf<16>(n, es, g, q, deg);
  }
  u32 top= 0, bt= 0, cc= 2 * n;
  for(u32 r= 0; r < n; ++r) {
   if(rec[r].key) continue;
   u32 v= r, nf= rec[r].first;
   auto c= g.rest(r);
   F* sp= fs;
   st[top++]= v, rec[v].key= top, bs[bt++]= top;
   for(;;) {
    u32 w;
    if(nf != NIL) w= nf, nf= NIL;
    else if(!R::done(c)) w= g.next(c);
    else {
     const u32 p= rec[v].key;
     if(p == bs[bt - 1]) {
      --bt;
      while(top >= p) rec[st[--top]].key= cc;
      --cc;
     }
     if(sp == fs) break;
     --sp, v= sp->v, c= sp->c;
     continue;
    }
    const auto rw= rec[w];
    if(!rw.key) *sp++= {v, c}, v= w, nf= rw.first, c= g.rest(w), st[top++]= v, rec[v].key= top, bs[bt++]= top;
    else
     while(rw.key < bs[bt - 1]) --bt;
   }
  }
  K= P + (2 * n - cc), base= cc + 1;
 }
 int comp(int v) const {
  const u32 x= g.rec[v].key;
  return int(x > 2 * n ? NIL - x : x - base + P);
 }
};

template <class R, bool HP> using PearceRecDFS= PearceRec<R, HP, 0>;
template <class R, bool HP> using PearceRecTrim= PearceRec<R, HP, 1>;
template <class R, bool HP> using PearceRecTrimPf= PearceRec<R, HP, 2>;
template <class R, bool HP> using GabowRecDFS= GabowRec<R, HP, 0>;
template <class R, bool HP> using GabowRecTrim= GabowRec<R, HP, 1>;
template <class R, bool HP> using GabowRecTrimPf= GabowRec<R, HP, 2>;
}
