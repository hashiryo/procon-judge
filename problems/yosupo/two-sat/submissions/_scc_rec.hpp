#pragma once
// yosupo-scc の同じ名前の核を 2026-10-11 に写したもの。yosupo-two-sat の gabow_* の 5 本が使う。2-SAT に合わせた手は
// ここを直さず、別のファイルに書く (2 問は別々に詰める)。
// 頂点ごとの記録に、方式の鍵 (Pearce の rix、path-based の I) と最初の隣接を並べて置く形。_scc.hpp の形では、頂点 w に
// 下りると、鍵と隣接の始まりを読み、その値で隣接の先頭を読んでから次の頂点に進むので、下りる鎖の 1 歩に読み込みが 2 段
// つながる。最初の隣接を鍵と同じ記録に置けば、1 歩が読み込み 1 段で済む。残りの隣接は、R = RecCSR なら CSR (最初の隣接も
// 並んでいて、そこは飛ばす)、R = RecList なら記録に先頭を持つ辺の連結リストから読み、読み始めるのは最初の隣接から帰ったあと。
//
// TRIM が 1 以上なら、DFS の前に入次数 0 の頂点を待ち行列で剥がす (Kahn)。剥がした頂点はどれも 1 頂点の成分で、剥がした
// 順がトポロジカル順の先頭に来る。剥がされない頂点から剥がした頂点への辺は無いので、DFS は剥がした頂点に触れない。DFS は
// 読み込みが 1 本の鎖につながるが、剥がしは待ち行列の頂点ごとに独立なので、読み込みを並べて出せる。剥がした頂点の鍵は
// NIL - (剥がした順) にして、DFS の根の走査で飛ばす。TRIM が 1 なら入次数を別の走査で数えて素直に剥がし、2 なら剥がしで
// 待ち行列の先の頂点を先読みする (peel_pf)。3 は 2 の入次数を、隣接を組む走査の中で数える。4 は 3 の先読みの位置を待ち行列の
// 末尾から切り離し、2 番目の隣接も分岐せずに扱う (peel_pf2)。5 は 4 に加えて、DFS の根の走査を記録でなく入次数の配列で行う
// (剥がした頂点は入次数が 0、残った頂点は 0 でない)。
//
// 成分の番号の振り方は _scc.hpp の同じ名前の方式と同じで、TRIM なら剥がした頂点の分だけ後ろへずらす。
// run() は段 (alloc、build、count、peel、dfs) を順に呼ぶだけ。辺の列は Edges でなくてもよく、size() と添字で辺 {a, b} を返し、
// 範囲 for で回せる型ならよい。
#include "_scc.hpp"
namespace scc {
// 記録は鍵と最初の隣接の 8 byte。残りは CSR。CSR を組んでから、頂点の順に 1 回なめて記録を作る。
// BL なら、記録を作る走査の「隣接が無ければ NIL」を分岐にせず、adj[off[v]] を毎回読んでマスクで選ぶ。adj は 2 語多く取り、
// 範囲の外を読んでもよいようにしておく (peel_pf2 も off[v] + 1 を読む)。
template <bool BL> struct RecCSRT {
 struct Rec {
  u32 key, first;
 };
 Rec* rec;
 u32 *off, *adj;
 struct Cur {
  u32 i, e;
 };
 template <class M> static size_t words(size_t n, size_t m) { return M::words(2 * n) + M::words(n + 1) + M::words(m + 2); }
 template <class M> void alloc(int n, size_t m, M& mem) { rec= mem.template take_as<Rec>(n), off= mem.take(n + 1), adj= mem.take(m + 2); }
 // deg が nullptr でなければ、次数を数える走査で入次数も数える (deg は 0 で埋めてから渡す)。
 template <class E> void build(int n, const E& es, u32* deg) {
  const size_t m= es.size();
  memset(off, 0, (n + 1) * sizeof(u32));
  if(deg)
   for(const auto& e: es) ++off[e[0]], ++deg[e[1]];
  else
   for(const auto& e: es) ++off[e[0]];
  for(int i= 1; i <= n; ++i) off[i]+= off[i - 1];
  for(size_t i= m; i--;) {
   const auto e= es[i];
   adj[--off[e[0]]]= e[1];
  }
  if constexpr(BL) {
   for(int v= 0; v < n; ++v) {
    const u32 a= off[v], f= adj[a], k= -u32(a < off[v + 1]);
    rec[v]= {0, (f & k) | ~k};
   }
  } else
   for(int v= 0; v < n; ++v) rec[v]= {0, off[v] < off[v + 1] ? adj[off[v]] : NIL};
 }
 u32& key(u32 v) { return rec[v].key; }
 u32 key(u32 v) const { return rec[v].key; }
 u32 first(u32 v) const { return rec[v].first; }
 // 最初の隣接は記録から読むので、残りは off[v] + 1 から。隣接が無ければ空。
 Cur rest(u32 v) const {
  const u32 e= off[v + 1];
  return {min(off[v] + 1, e), e};
 }
 static bool done(const Cur& c) { return c.i == c.e; }
 u32 next(Cur& c) const { return adj[c.i++]; }
};
// 記録は鍵と最初の隣接と残りの連結リストの先頭の 16 byte。辺の列を 1 回前から読むだけで組める。
// BL なら、組む走査の「その頂点の最初の辺か」を分岐にせず、マスクで選ぶ。最初の辺でも nx の辺の場所には書く (使われない)。
// 辺 m は peel_pf2 の番兵で、行き先が n、次が NIL。
template <bool BL> struct RecListT {
 struct Rec {
  u32 key, first, head, pad;
 };
 Rec* rec;
 u32* nx;
 u32 m;
 struct Cur {
  u32 e;
 };
 template <class M> static size_t words(size_t n, size_t m) { return M::words(4 * n) + M::words(2 * m + 2); }
 template <class M> void alloc(int n, size_t m_, M& mem) { rec= mem.template take_as<Rec>(n), nx= mem.take(2 * m_ + 2), m= m_; }
 template <bool C, class E> void link(const E& es, u32* deg) {
  for(size_t i= 0; i < es.size(); ++i) {
   const auto e= es[i];
   const u32 b= e[1];
   Rec& r= rec[e[0]];
   if constexpr(BL) {
    const u32 f= r.first, h= r.head, k= -u32(f == NIL);
    nx[2 * i]= b, nx[2 * i + 1]= h;
    r.first= (b & k) | (f & ~k), r.head= (h & k) | (u32(i) & ~k);
   } else if(r.first == NIL) r.first= b;
   else nx[2 * i]= b, nx[2 * i + 1]= r.head, r.head= i;
   if constexpr(C) ++deg[b];
  }
 }
 // deg が nullptr でなければ、同じ走査で入次数も数える (deg は 0 で埋めてから渡す)。
 template <class E> void build(int n, const E& es, u32* deg) {
  for(int v= 0; v < n; ++v) rec[v]= {0, NIL, NIL, 0};
  nx[2 * m]= n, nx[2 * m + 1]= NIL;
  if(deg) link<true>(es, deg);
  else link<false>(es, deg);
 }
 u32& key(u32 v) { return rec[v].key; }
 u32 key(u32 v) const { return rec[v].key; }
 u32 first(u32 v) const { return rec[v].first; }
 Cur rest(u32 v) const { return {rec[v].head}; }
 static bool done(const Cur& c) { return c.e == NIL; }
 u32 next(Cur& c) const {
  const u32 w= nx[2 * c.e];
  c.e= nx[2 * c.e + 1];
  return w;
 }
};
// 鍵、最初の隣接、残りの連結リストの先頭を、別々の 4 byte の配列 (ky、fst、hd) に置く形。DFS の 1 歩の鎖は fst の読み込み 1 段の
// ままで、鍵は、まだ訪れていないという分岐の予想が当たるあいだは鎖に入らない。RecListT の 16 byte の記録より配列が小さいので、
// L2 の大きい CPU では載りやすい。組む走査は RecListB と同じくマスクで選ぶが、離れた場所の読み書きは fst と hd の 2 か所になる。
// 辺 m は peel の番兵で、行き先が n、次が NIL。
struct RecSplit {
 u32 *ky, *fst, *hd, *nx;
 u32 m;
 struct Cur {
  u32 e;
 };
 template <class M> static size_t words(size_t n, size_t m) { return 3 * M::words(n) + M::words(2 * m + 2); }
 template <class M> void alloc(int n, size_t m_, M& mem) { ky= mem.take(n), fst= mem.take(n), hd= mem.take(n), nx= mem.take(2 * m_ + 2), m= m_; }
 template <bool C, class E> void link(const E& es, u32* deg) {
  for(size_t i= 0; i < es.size(); ++i) {
   const auto ed= es[i];
   const u32 a= ed[0], b= ed[1], f= fst[a], h= hd[a], k= -u32(f == NIL);
   nx[2 * i]= b, nx[2 * i + 1]= h;
   fst[a]= (b & k) | (f & ~k), hd[a]= (h & k) | (u32(i) & ~k);
   if constexpr(C) ++deg[b];
  }
 }
 template <class E> void build(int n, const E& es, u32* deg) {
  memset(ky, 0, n * sizeof(u32)), memset(fst, 0xff, n * sizeof(u32)), memset(hd, 0xff, n * sizeof(u32));
  nx[2 * m]= n, nx[2 * m + 1]= NIL;
  if(deg) link<true>(es, deg);
  else link<false>(es, deg);
 }
 u32& key(u32 v) { return ky[v]; }
 u32 key(u32 v) const { return ky[v]; }
 u32 first(u32 v) const { return fst[v]; }
 Cur rest(u32 v) const { return {hd[v]}; }
 static bool done(const Cur& c) { return c.e == NIL; }
 u32 next(Cur& c) const {
  const u32 w= nx[2 * c.e];
  c.e= nx[2 * c.e + 1];
  return w;
 }
};
// 入次数を数える。deg は n + 1 語で、deg[n] は peel_pf の番兵。
template <class E> void count_in(int n, const E& es, u32* deg) {
 memset(deg, 0, (n + 1) * sizeof(u32));
 for(const auto& e: es) ++deg[e[1]];
}
// 入次数 0 の頂点を剥がし、剥がした数を返す。剥がした頂点の鍵は NIL - (剥がした順) にする。q は n + 1 語。
// 積むかどうかは分岐にせず、毎回 q の末尾に書いて、入次数が 0 になったときだけ末尾を進める。
template <class R> u32 peel_plain(int n, R& g, u32* q, u32* deg) {
 u32 qt= 0;
 for(u32 v= 0; v < u32(n); ++v) q[qt]= v, qt+= deg[v] == 0;
 for(u32 qh= 0; qh < qt; ++qh) {
  const u32 v= q[qh];
  g.key(v)= NIL - qh;
  u32 w= g.first(v);
  if(w == NIL) continue;
  q[qt]= w, qt+= --deg[w] == 0;
  for(auto c= g.rest(v); !R::done(c);) w= g.next(c), q[qt]= w, qt+= --deg[w] == 0;
 }
 return qt;
}
// peel_plain と同じことを、待ち行列の D 個先の頂点の記録と隣接の始まり、D / 2 個先の頂点の最初の隣接の入次数と残りの隣接を
// 先読みしながら行う。読み込みの結果で決まる分岐は、予想を外すとその読み込みの待ちがそのまま出るので、先読みで待ちを
// 短くする。最初の隣接が無い頂点は、行き先を番兵の n にして同じ手順を踏む (deg[n] は 0 から減らすので 0 に戻らず、積まれない)。
// 待ち行列の末尾より先はまだ決まっていないので、先読みの位置は末尾で止める。
template <u32 D, bool BL> u32 peel_pf(int n, RecCSRT<BL>& g, u32* q, u32* deg) {
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
// 連結リストの形。残りの隣接は D / 2 個先で先頭の辺だけを先読みする (その先は辺をたどらないと分からない)。
template <u32 D, bool BL> u32 peel_pf(int n, RecListT<BL>& g, u32* q, u32* deg) {
 u32 qt= 0;
 for(u32 v= 0; v < u32(n); ++v) q[qt]= v, qt+= deg[v] == 0;
 auto* rec= g.rec;
 const u32* nx= g.nx;
 for(u32 qh= 0; qh < qt; ++qh) {
  __builtin_prefetch(rec + q[min(qh + D, qt - 1)]);
  const auto& rb= rec[q[min(qh + D / 2, qt - 1)]];
  __builtin_prefetch(deg + min(rb.first, u32(n))), __builtin_prefetch(nx + 2 * (rb.head == NIL ? 0 : rb.head));
  const u32 v= q[qh];
  auto& r= rec[v];
  r.key= NIL - qh;
  u32 w= min(r.first, u32(n));
  q[qt]= w, qt+= --deg[w] == 0;
  for(u32 e= r.head; e != NIL; e= nx[2 * e + 1]) w= nx[2 * e], q[qt]= w, qt+= --deg[w] == 0;
 }
 return qt;
}
// RecSplit の形。先読みする場所は RecListT の peel_pf と同じで、記録が 2 つの配列に分かれている。
template <u32 D> u32 peel_pf(int n, RecSplit& g, u32* q, u32* deg) {
 u32 qt= 0;
 for(u32 v= 0; v < u32(n); ++v) q[qt]= v, qt+= deg[v] == 0;
 const u32 *fst= g.fst, *hd= g.hd, *nx= g.nx;
 u32* ky= g.ky;
 for(u32 qh= 0; qh < qt; ++qh) {
  const u32 a= q[min(qh + D, qt - 1)];
  __builtin_prefetch(fst + a), __builtin_prefetch(hd + a), __builtin_prefetch(ky + a);
  const u32 b= q[min(qh + D / 2, qt - 1)], hb= hd[b];
  __builtin_prefetch(deg + min(fst[b], u32(n))), __builtin_prefetch(nx + 2 * (hb == NIL ? 0 : hb));
  const u32 v= q[qh];
  ky[v]= NIL - qh;
  u32 w= min(fst[v], u32(n));
  q[qt]= w, qt+= --deg[w] == 0;
  for(u32 e= hd[v]; e != NIL; e= nx[2 * e + 1]) w= nx[2 * e], q[qt]= w, qt+= --deg[w] == 0;
 }
 return qt;
}
// peel_pf の先読みの位置を、待ち行列の末尾から切り離したもの。peel_pf は位置を末尾で止めていたので、先読みの番地が、直前までの
// 入次数の読み込みで決まる末尾を待っていた。ここでは末尾に関係なく q[qh + D] を読み、値を n - 1 で止めてから先読みする (まだ
// 積まれていない場所の値でも、番地が範囲に入っていれば害は無い。q は n + 1 + D 語取る)。2 番目の隣接も、無ければ番兵の n を
// 行き先にして分岐せずに入次数を減らし、その入次数も D / 4 個先で先読みする。3 番目からは分岐で回す。
template <u32 D, bool BL> u32 peel_pf2(int n, RecCSRT<BL>& g, u32* q, u32* deg) {
 u32 qt= 0;
 for(u32 v= 0; v < u32(n); ++v) q[qt]= v, qt+= deg[v] == 0;
 auto* rec= g.rec;
 const u32 *off= g.off, *adj= g.adj, N= n, L= n - 1;
 for(u32 qh= 0; qh < qt; ++qh) {
  const u32 a= min(q[qh + D], L);
  __builtin_prefetch(rec + a), __builtin_prefetch(off + a);
  const u32 b= min(q[qh + D / 2], L);
  __builtin_prefetch(deg + min(rec[b].first, N)), __builtin_prefetch(adj + off[b] + 1);
  __builtin_prefetch(deg + min(adj[off[min(q[qh + D / 4], L)] + 1], N));
  const u32 v= q[qh];
  auto& r= rec[v];
  r.key= NIL - qh;
  u32 w= min(r.first, N);
  q[qt]= w, qt+= --deg[w] == 0;
  const u32 i= off[v] + 1, e= off[v + 1], k= -u32(i < e);
  w= (adj[i] & k) | (N & ~k);
  q[qt]= w, qt+= --deg[w] == 0;
  for(u32 j= i + 1; j < e; ++j) w= adj[j], q[qt]= w, qt+= --deg[w] == 0;
 }
 return qt;
}
// 連結リストの形。2 番目の隣接は、残りが無ければ番兵の辺 m を読む。
template <u32 D, bool BL> u32 peel_pf2(int n, RecListT<BL>& g, u32* q, u32* deg) {
 u32 qt= 0;
 for(u32 v= 0; v < u32(n); ++v) q[qt]= v, qt+= deg[v] == 0;
 auto* rec= g.rec;
 const u32 *nx= g.nx, N= n, L= n - 1, M= g.m;
 for(u32 qh= 0; qh < qt; ++qh) {
  __builtin_prefetch(rec + min(q[qh + D], L));
  const auto& rb= rec[min(q[qh + D / 2], L)];
  __builtin_prefetch(deg + min(rb.first, N)), __builtin_prefetch(nx + 2 * min(rb.head, M));
  __builtin_prefetch(deg + min(nx[2 * min(rec[min(q[qh + D / 4], L)].head, M)], N));
  const u32 v= q[qh];
  auto& r= rec[v];
  r.key= NIL - qh;
  u32 w= min(r.first, N);
  q[qt]= w, qt+= --deg[w] == 0;
  u32 e= min(r.head, M);
  w= nx[2 * e];
  q[qt]= w, qt+= --deg[w] == 0;
  for(e= nx[2 * e + 1]; e != NIL; e= nx[2 * e + 1]) w= nx[2 * e], q[qt]= w, qt+= --deg[w] == 0;
 }
 return qt;
}
// 段の共通部分。方式ごとの DFS は派生側の dfs() に書く。
template <class R, bool HP, int TRIM, class F> struct RecBase {
 using M= Mem<HP>;
 M mem;
 R g;
 u32 n, K, base, P= 0;
 u32 *st, *aux, *q= nullptr, *deg= nullptr;
 F* fs;
 void alloc(int n_, size_t m) {
  n= n_;
  mem.reserve(R::template words<M>(n, m) + 2 * M::words(n) + M::words(n * sizeof(F) / sizeof(u32)) + (TRIM ? M::words(n + 33) + M::words(n + 1) : 0));
  g.alloc(n, m, mem);
  st= mem.take(n), aux= mem.take(n), fs= mem.template take_as<F>(n);
  if constexpr(TRIM) q= mem.take(n + 33), deg= mem.take(n + 1);
 }
 template <class E> void build(const E& es) {
  if constexpr(TRIM >= 3) memset(deg, 0, (n + 1) * sizeof(u32)), g.build(n, es, deg);
  else g.build(n, es, nullptr);
 }
 template <class E> void count(const E& es) {
  if constexpr(TRIM == 1 || TRIM == 2) count_in(n, es, deg);
 }
 void peel() {
  if constexpr(TRIM == 1) P= peel_plain(n, g, q, deg);
  else if constexpr(TRIM == 2 || TRIM == 3) P= peel_pf<16>(n, g, q, deg);
  else if constexpr(TRIM >= 4) P= peel_pf2<16>(n, g, q, deg);
 }
};
// Pearce の省メモリ版 (_scc.hpp の Pearce と同じ) を記録の形で回す。nf は、いまの頂点の最初の隣接のうち、まだ見ていないもの。
template <class R> struct PearceFrame {
 u32 v, lv, ov;
 typename R::Cur c;
};
template <class R, bool HP, int TRIM> struct PearceRec: RecBase<R, HP, TRIM, PearceFrame<R>> {
 using B= RecBase<R, HP, TRIM, PearceFrame<R>>;
 using B::g, B::n, B::K, B::base, B::P, B::st, B::fs;
 void dfs() {
  u32 index= 1, cc= n - 1, top= 0;
  for(u32 r= 0; r < n; ++r) {
   if constexpr(TRIM == 5)
    if(!this->deg[r]) continue;
   if(g.key(r)) continue;
   u32 v= r, ov= index++, lv= ov, nf= g.first(r);
   auto c= g.rest(r);
   auto* sp= fs;
   g.key(v)= ov;
   for(;;) {
    u32 w;
    if(nf != NIL) w= nf, nf= NIL;
    else if(!R::done(c)) w= g.next(c);
    else {
     u32 x;
     if(lv == ov) {
      --index;
      while(top && ov <= g.key(st[top - 1])) g.key(st[--top])= cc, --index;
      x= g.key(v)= cc--;
     } else x= g.key(v)= lv, st[top++]= v;
     if(sp == fs) break;
     --sp, v= sp->v, ov= sp->ov, lv= sp->lv, c= sp->c;
     if(x < lv) lv= x;
     continue;
    }
    const u32 kw= g.key(w), fw= g.first(w);
    if(!kw) *sp++= {v, lv, ov, c}, v= w, ov= lv= index++, g.key(v)= ov, nf= fw, c= g.rest(w);
    else if(kw < lv) lv= kw;
   }
  }
  K= P + (n - 1 - cc), base= cc + 1;
 }
 template <class E> void run(int n_, const E& es) { this->alloc(n_, es.size()), this->build(es), this->count(es), this->peel(), dfs(); }
 int comp(int v) const {
  const u32 x= g.key(v);
  return int(x > n ? NIL - x : x - base + P);
 }
};
// path-based (_scc.hpp の Gabow と同じ) を記録の形で回す。根の候補の位置のスタックは aux に置く。
template <class R> struct GabowFrame {
 u32 v;
 typename R::Cur c;
};
template <class R, bool HP, int TRIM> struct GabowRec: RecBase<R, HP, TRIM, GabowFrame<R>> {
 using B= RecBase<R, HP, TRIM, GabowFrame<R>>;
 using B::g, B::n, B::K, B::base, B::P, B::st, B::fs;
 void dfs() {
  u32* bs= this->aux;
  u32 top= 0, bt= 0, cc= 2 * n;
  for(u32 r= 0; r < n; ++r) {
   if constexpr(TRIM == 5)
    if(!this->deg[r]) continue;
   if(g.key(r)) continue;
   u32 v= r, nf= g.first(r);
   auto c= g.rest(r);
   auto* sp= fs;
   st[top++]= v, g.key(v)= top, bs[bt++]= top;
   for(;;) {
    u32 w;
    if(nf != NIL) w= nf, nf= NIL;
    else if(!R::done(c)) w= g.next(c);
    else {
     const u32 p= g.key(v);
     if(p == bs[bt - 1]) {
      --bt;
      while(top >= p) g.key(st[--top])= cc;
      --cc;
     }
     if(sp == fs) break;
     --sp, v= sp->v, c= sp->c;
     continue;
    }
    const u32 kw= g.key(w), fw= g.first(w);
    if(!kw) *sp++= {v, c}, v= w, nf= fw, c= g.rest(w), st[top++]= v, g.key(v)= top, bs[bt++]= top;
    else
     while(kw < bs[bt - 1]) --bt;
   }
  }
  K= P + (2 * n - cc), base= cc + 1;
 }
 template <class E> void run(int n_, const E& es) { this->alloc(n_, es.size()), this->build(es), this->count(es), this->peel(), dfs(); }
 int comp(int v) const {
  const u32 x= g.key(v);
  return int(x > 2 * n ? NIL - x : x - base + P);
 }
};
using RecCSR= RecCSRT<false>;
using RecCSRB= RecCSRT<true>;
using RecList= RecListT<false>;
using RecListB= RecListT<true>;
template <class R, bool HP> using PearceRecDFS= PearceRec<R, HP, 0>;
template <class R, bool HP> using PearceRecTrim= PearceRec<R, HP, 1>;
template <class R, bool HP> using PearceRecTrimPf= PearceRec<R, HP, 2>;
template <class R, bool HP> using PearceRecTrimPfFused= PearceRec<R, HP, 3>;
template <class R, bool HP> using PearceRecTrimPf2Fused= PearceRec<R, HP, 4>;
template <class R, bool HP> using PearceRecTrimPf2FusedDs= PearceRec<R, HP, 5>;
template <class R, bool HP> using GabowRecDFS= GabowRec<R, HP, 0>;
template <class R, bool HP> using GabowRecTrim= GabowRec<R, HP, 1>;
template <class R, bool HP> using GabowRecTrimPf= GabowRec<R, HP, 2>;
template <class R, bool HP> using GabowRecTrimPfFused= GabowRec<R, HP, 3>;
template <class R, bool HP> using GabowRecTrimPf2Fused= GabowRec<R, HP, 4>;
template <class R, bool HP> using GabowRecTrimPf2FusedDs= GabowRec<R, HP, 5>;
// 平均の出次数で隣接の持ち方を選ぶ。m >= 2 n なら CSR、そうでなければ連結リスト。連結リストは組むのが速く、辺の少ない
// グラフで勝つが、たどるときは辺ごとに離れた場所を読むので、密なグラフでは CSR に負ける (1 回目の random_02)。
template <template <class, bool> class A, bool HP> struct AutoAlg {
 bool csr= false;
 A<RecCSR, HP> c;
 A<RecList, HP> l;
 u32 K= 0;
 template <class E> void run(int n, const E& es) {
  csr= es.size() >= 2 * size_t(n);
  if(csr) c.run(n, es), K= c.K;
  else l.run(n, es), K= l.K;
 }
 int comp(int v) const { return csr ? c.comp(v) : l.comp(v); }
};
template <template <class, bool> class A, bool HP> struct AutoSolver {
 int n;
 const Edges& es;
 AutoAlg<A, HP> a;
 AutoSolver(int n, const Edges& es): n(n), es(es) {}
 void run() { a.run(n, es); }
 int count() const { return a.K; }
 int comp(int v) const { return a.comp(v); }
};
}
