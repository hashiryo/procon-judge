#pragma once
// yosupo-scc の同じ名前の核を 2026-10-11 に写したもの。yosupo-two-sat の gabow_* の 5 本が使う。2-SAT に合わせた手は
// ここを直さず、別のファイルに書く (2 問は別々に詰める)。
// 強連結成分分解の核。方式 (Kosaraju、Tarjan、Pearce、path-based) と、隣接の持ち方 (CSR、辺の連結リスト) と、
// 作業領域の確保 (配列ごとの malloc、2 MB 境界の 1 本の領域で huge page を頼む形) を掛け合わせて提出を作る。
// DFS はどれも反復で書き、いま見ている頂点とその辺の位置は変数に置いて、下りるときだけフレームのスタックに積む。
// 成分の番号は run() の中では方式ごとの生の値で持ち、comp(v) で 0 から K - 1 のトポロジカル順に直して返す。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace scc {
using Edges= vector<array<int, 2>>;
constexpr u32 NIL= ~u32(0);
// 作業領域。HP なら reserve で全体を 2 MB 境界の 1 本の領域に取り、huge page を頼む。take はそこから 64 byte 境界に揃えて
// 切り出す。HP でなければ take ごとに malloc する。どちらも 0 では埋めない。フレームのスタックのように最悪の大きさで取って
// 一部しか触らない配列があるので、触る前にまとめて用意させること (MADV_POPULATE_WRITE) はしない。
template <bool HP> struct Mem {
 vector<void*> blocks;
 u32* base= nullptr;
 size_t used= 0, bytes= 0;  // bytes は HP のときに取った大きさ
 Mem()= default;
 Mem(const Mem&)= delete;
 ~Mem() {
  for(void* p: blocks) free(p);
 }
 // reserve に渡す語数を数えるときは、配列ごとにこれで切り上げる。
 static constexpr size_t words(size_t n) { return (n + 15) & ~size_t(15); }
 void reserve(size_t w) {
  if constexpr(HP) {
   constexpr size_t H= size_t(1) << 21;
   bytes= (w * sizeof(u32) + H - 1) & ~(H - 1);
   base= static_cast<u32*>(aligned_alloc(H, bytes));
#ifdef __linux__
   madvise(base, bytes, MADV_HUGEPAGE);
#endif
   blocks.push_back(base);
  }
 }
 u32* take(size_t n) {
  if constexpr(HP) {
   u32* p= base + used;
   used+= words(n);
   return p;
  } else {
   void* p= malloc(words(n) * sizeof(u32));
   blocks.push_back(p);
   return static_cast<u32*>(p);
  }
 }
 template <class T> T* take_as(size_t n) { return reinterpret_cast<T*>(take(n * sizeof(T) / sizeof(u32))); }
};
// CSR。頂点 v から出る辺の行き先は adj[off[v]] から adj[off[v + 1] - 1]。S = 1 なら辺を逆向きに読む。
// 次数を数え、累積を取り、辺を後ろから置く (Library の Graph::adjacency_vertex と同じ組み方)。
struct CSR {
 u32 *off, *adj;
 struct Cur {
  u32 i, e;
 };
 template <class M> static size_t words(size_t n, size_t m) { return M::words(n + 1) + M::words(m); }
 template <int S, class M> void build(int n, const Edges& es, M& mem) {
  const size_t m= es.size();
  off= mem.take(n + 1), adj= mem.take(m);
  memset(off, 0, (n + 1) * sizeof(u32));
  for(const auto& e: es) ++off[e[S]];
  for(int i= 1; i <= n; ++i) off[i]+= off[i - 1];
  for(size_t i= m; i--;) adj[--off[es[i][S]]]= es[i][!S];
 }
 Cur first(u32 v) const { return {off[v], off[v + 1]}; }
 static bool done(const Cur& c) { return c.i == c.e; }
 u32 to(const Cur& c) const { return adj[c.i]; }
 void step(Cur& c) const { ++c.i; }
};
// 辺の連結リスト。nx[2 e] が辺 e の行き先、nx[2 e + 1] が同じ頂点から出る次の辺 (NIL で終わり)。辺の列を 1 回
// 前から読むだけで組める。たどるときは辺ごとに 1 か所を読む。
struct List {
 u32 *head, *nx;
 struct Cur {
  u32 e;
 };
 template <class M> static size_t words(size_t n, size_t m) { return M::words(n) + M::words(2 * m); }
 template <int S, class M> void build(int n, const Edges& es, M& mem) {
  const size_t m= es.size();
  head= mem.take(n), nx= mem.take(2 * m);
  memset(head, 0xff, n * sizeof(u32));
  for(size_t i= 0; i < m; ++i) {
   const u32 a= es[i][S];
   nx[2 * i]= es[i][!S], nx[2 * i + 1]= head[a], head[a]= i;
  }
 }
 Cur first(u32 v) const { return {head[v]}; }
 static bool done(const Cur& c) { return c.e == NIL; }
 u32 to(const Cur& c) const { return nx[2 * c.e]; }
 void step(Cur& c) const { c.e= nx[2 * c.e + 1]; }
};
// Kosaraju。順向きの DFS の帰りがけ順を取り、その逆順に、逆向きのグラフでまだ成分の無い頂点を集める。見つかる順が
// そのままトポロジカル順になる。cm[v] は 0 が未訪問、1 が順向きで訪問済み、2 + k が成分 k。
template <class G, bool HP> struct Kosaraju {
 using M= Mem<HP>;
 struct F {
  u32 v;
  typename G::Cur c;
 };
 M mem;
 u32* cm;
 u32 K;
 void run(int n, const Edges& es) {
  const size_t m= es.size();
  mem.reserve(2 * G::template words<M>(n, m) + 3 * M::words(n) + M::words(n * sizeof(F) / sizeof(u32)));
  G g, h;
  g.template build<0>(n, es, mem), h.template build<1>(n, es, mem);
  cm= mem.take(n);
  u32 *post= mem.take(n), *st= mem.take(n);
  F* fs= mem.template take_as<F>(n);
  memset(cm, 0, n * sizeof(u32));
  u32 k= 0;
  for(u32 r= 0; r < u32(n); ++r) {
   if(cm[r]) continue;
   cm[r]= 1;
   u32 v= r;
   auto c= g.first(r);
   F* sp= fs;
   for(;;) {
    if(!G::done(c)) {
     const u32 w= g.to(c);
     g.step(c);
     if(!cm[w]) cm[w]= 1, *sp++= {v, c}, v= w, c= g.first(w);
    } else {
     post[k++]= v;
     if(sp == fs) break;
     --sp, v= sp->v, c= sp->c;
    }
   }
  }
  u32 id= 2;
  for(u32 i= n; i--;) {
   const u32 r= post[i];
   if(cm[r] != 1) continue;
   cm[r]= id;
   u32 top= 0;
   st[top++]= r;
   while(top) {
    const u32 x= st[--top];
    for(auto c= h.first(x); !G::done(c); h.step(c)) {
     const u32 w= h.to(c);
     if(cm[w] == 1) cm[w]= id, st[top++]= w;
    }
   }
   ++id;
  }
  K= id - 2;
 }
 int comp(int v) const { return int(cm[v] - 2); }
};
// Tarjan。訪問順 ord と lowlink の low を頂点ごとの配列で持つ。成分に入った頂点は ord を NIL - k にして、スタックに
// 載っているかの印と成分の番号を兼ねる (どの訪問順より大きいので low を下げない)。成分はトポロジカル順の逆に見つかる。
template <class G, bool HP> struct Tarjan {
 using M= Mem<HP>;
 struct F {
  u32 v;
  typename G::Cur c;
 };
 M mem;
 u32* ord;
 u32 K;
 void run(int n, const Edges& es) {
  const size_t m= es.size();
  mem.reserve(G::template words<M>(n, m) + 3 * M::words(n) + M::words(n * sizeof(F) / sizeof(u32)));
  G g;
  g.template build<0>(n, es, mem);
  ord= mem.take(n);
  u32 *low= mem.take(n), *st= mem.take(n);
  F* fs= mem.template take_as<F>(n);
  memset(ord, 0, n * sizeof(u32));
  u32 idx= 0, top= 0, k= 0;
  for(u32 r= 0; r < u32(n); ++r) {
   if(ord[r]) continue;
   u32 v= r;
   auto c= g.first(r);
   F* sp= fs;
   ord[v]= low[v]= ++idx, st[top++]= v;
   for(;;) {
    if(!G::done(c)) {
     const u32 w= g.to(c);
     g.step(c);
     const u32 ow= ord[w];
     if(!ow) *sp++= {v, c}, v= w, c= g.first(w), ord[v]= low[v]= ++idx, st[top++]= v;
     else if(ow < low[v]) low[v]= ow;
    } else {
     const u32 lv= low[v];
     if(lv == ord[v]) {
      u32 x;
      do x= st[--top], ord[x]= NIL - k;
      while(x != v);
      ++k;
     }
     if(sp == fs) break;
     --sp, v= sp->v, c= sp->c;
     if(lv < low[v]) low[v]= lv;
    }
   }
  }
  K= k;
 }
 int comp(int v) const { return int(K - 1 - (NIL - ord[v])); }
};
// Pearce の省メモリ版。頂点ごとの配列は rix の 1 本だけで、訪問中は訪問順、帰ったあとは下がった lowlink、成分に入ったら
// 成分の番号 (n - 1 から下がる) を持つ。成分に入れた頂点の数だけ訪問順を戻すので、訪問中の値はどの成分の番号より小さく、
// 成分に入った頂点は lowlink を下げない。訪問中の頂点の lowlink と訪問順はフレームに持ち、根かどうかは lowlink が訪問順
// から下がっていないかで見る。成分はトポロジカル順の逆に、大きい番号から振られる。
template <class G, bool HP> struct Pearce {
 using M= Mem<HP>;
 struct F {
  u32 v, lv, ov;
  typename G::Cur c;
 };
 M mem;
 u32* rix;
 u32 K, base;
 void run(int n, const Edges& es) {
  const size_t m= es.size();
  mem.reserve(G::template words<M>(n, m) + 2 * M::words(n) + M::words(n * sizeof(F) / sizeof(u32)));
  G g;
  g.template build<0>(n, es, mem);
  rix= mem.take(n);
  u32* st= mem.take(n);
  F* fs= mem.template take_as<F>(n);
  memset(rix, 0, n * sizeof(u32));
  u32 index= 1, cc= n - 1, top= 0;
  for(u32 r= 0; r < u32(n); ++r) {
   if(rix[r]) continue;
   u32 v= r, ov= index++, lv= ov;
   auto c= g.first(r);
   F* sp= fs;
   rix[v]= ov;
   for(;;) {
    if(!G::done(c)) {
     const u32 w= g.to(c);
     g.step(c);
     const u32 rw= rix[w];
     if(!rw) *sp++= {v, lv, ov, c}, v= w, ov= lv= index++, rix[v]= ov, c= g.first(w);
     else if(rw < lv) lv= rw;
    } else {
     u32 x;
     if(lv == ov) {
      --index;
      while(top && ov <= rix[st[top - 1]]) rix[st[--top]]= cc, --index;
      x= rix[v]= cc--;
     } else x= rix[v]= lv, st[top++]= v;
     if(sp == fs) break;
     --sp, v= sp->v, ov= sp->ov, lv= sp->lv, c= sp->c;
     if(x < lv) lv= x;
    }
   }
  }
  K= n - 1 - cc, base= cc + 1;
 }
 int comp(int v) const { return int(rix[v] - base); }
};
// path-based (Gabow)。頂点ごとの配列は I の 1 本で、st に載っている間はその位置 (1 から)、成分に入ったら成分の番号
// (2n から下がる。どの位置より大きい) を持つ。bs は根の候補の位置のスタックで、辺の先が st に載っていれば、その位置より
// 上の候補を捨てる。成分に入った頂点は位置より大きいので何も捨てない。成分はトポロジカル順の逆に見つかる。
template <class G, bool HP> struct Gabow {
 using M= Mem<HP>;
 struct F {
  u32 v;
  typename G::Cur c;
 };
 M mem;
 u32* I;
 u32 K, base;
 void run(int n, const Edges& es) {
  const size_t m= es.size();
  mem.reserve(G::template words<M>(n, m) + 3 * M::words(n) + M::words(n * sizeof(F) / sizeof(u32)));
  G g;
  g.template build<0>(n, es, mem);
  I= mem.take(n);
  u32 *st= mem.take(n), *bs= mem.take(n);
  F* fs= mem.template take_as<F>(n);
  memset(I, 0, n * sizeof(u32));
  u32 top= 0, bt= 0, cc= 2 * n;
  for(u32 r= 0; r < u32(n); ++r) {
   if(I[r]) continue;
   u32 v= r;
   auto c= g.first(r);
   F* sp= fs;
   st[top++]= v, I[v]= top, bs[bt++]= top;
   for(;;) {
    if(!G::done(c)) {
     const u32 w= g.to(c);
     g.step(c);
     const u32 iw= I[w];
     if(!iw) *sp++= {v, c}, v= w, c= g.first(w), st[top++]= v, I[v]= top, bs[bt++]= top;
     else
      while(iw < bs[bt - 1]) --bt;
    } else {
     const u32 p= I[v];
     if(p == bs[bt - 1]) {
      --bt;
      while(top >= p) I[st[--top]]= cc;
      --cc;
     }
     if(sp == fs) break;
     --sp, v= sp->v, c= sp->c;
    }
   }
  }
  K= 2 * n - cc, base= cc + 1;
 }
 int comp(int v) const { return int(I[v] - base); }
};
template <template <class, bool> class A, class G, bool HP> struct Solver {
 int n;
 const Edges& es;
 A<G, HP> a;
 Solver(int n, const Edges& es): n(n), es(es) {}
 void run() { a.run(n, es); }
 int count() const { return a.K; }
 int comp(int v) const { return a.comp(v); }
};
}
