#pragma once
// 二重連結成分の核。Hopcroft–Tarjan を反復で書き、頂点のスタックから成分を切り出す。隣接は CSR。訪問順 ord と lowlink の low を
// 頂点ごとの配列で持つ。子 v から親 p に帰るとき、low[v] >= ord[p] なら、スタックの v から上と p が 1 つの成分になる。親への
// 辺も low に入れてよい (low[v] <= ord[p] になるだけで、判定 low[v] >= ord[p] は変わらない) ので、親の辺を飛ばさずに済み、
// 多重辺もそのまま扱える。辺を持たない頂点は、それだけで 1 つの成分。成分は頂点の列をつなげた bv と、区切りの bo に置き、
// answer() で列の列に直す。
//
// PEEL なら、DFS の前に次数 1 の頂点を待ち行列で剥がす。剥がした頂点 v と、そのとき残っていた唯一の隣接 u は、それだけで 1 つの
// 成分 {v, u} (辺 vu は橋)。頂点ごとに次数と、残っている隣接の番号の XOR を持つので、u は XOR の値そのもので、隣接を組まずに
// 辺の列だけで剥がせる。剥がしで次数が 0 になった頂点は、もう剥がした頂点との成分に入っているので、新しい成分を作らない。
// 剥がしたあとの次数はそのまま 2-core の CSR の次数になるので、CSR は 2-core の辺だけで組む。作業領域は 2 MB 境界の 1 本の
// 領域で、huge page を頼む (yosupo-scc の _scc.hpp の Mem<true> と同じ)。
//
// REC なら、DFS を頂点ごとの記録 {CSR の始まり, 終わり, 最初の隣接, 2 番目の隣接} (16 byte) で回し、low は頂点ごとの配列に置かず
// フレームに持つ (low[v] を読み書きするのは、v がスタックに載っている間だけで、子から帰るときに親へ渡せば足りる)。無向グラフの
// 隣接の 1 つは親なので、2 つ置けば、次数 2 の頂点 (閉路の頂点) では DFS が下りる 1 歩が記録の読み込み 1 段で済む。フレームには
// 頂点と位置と low だけを持ち、戻るときに記録を読み直す。
//
// ALG = 2 は DFS を使わない Tarjan–Vishkin。任意の全域木 (ここでは BFS の森) の上で、行きがけ順の番号 pre と部分木の大きさ nd、
// 部分木の頂点から辺で届く pre の最小 lo と最大 hi を求め、木の辺 (par[v], v) を頂点 v で表して union-find でつなぐ。
//   (a) 先祖と子孫の関係に無い 2 頂点 u、w を結ぶ辺があれば、u と w をつなぐ。
//   (b) par[v] が根でなく、lo[v] < pre[par[v]] か hi[v] >= pre[par[v]] + nd[par[v]] なら、v と par[v] をつなぐ。
// つないだ組が 1 つの二重連結成分の木の辺で、その成分の頂点は、組の頂点と、組のいちばん上の頂点の親。pre と nd は BFS の順と
// 逆順の走査で、lo と hi は逆順の走査で求まるので、どの走査も頂点ごとに読み込みが独立で、DFS のような鎖ができない。lo と hi
// には親への辺も子への辺も入れてよい (親の pre は判定の不等号を満たさず、子の pre は部分木の範囲に入る) ので、木の辺を見分け
// なくてよく、多重辺もそのまま扱える。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace bcc {
struct Mem {
 u32* base= nullptr;
 size_t used= 0;
 Mem()= default;
 Mem(const Mem&)= delete;
 ~Mem() { free(base); }
 static constexpr size_t words(size_t n) { return (n + 15) & ~size_t(15); }
 void reserve(size_t w) {
  constexpr size_t H= size_t(1) << 21;
  const size_t bytes= (w * sizeof(u32) + H - 1) & ~(H - 1);
  base= static_cast<u32*>(aligned_alloc(H, bytes));
#ifdef __linux__
  madvise(base, bytes, MADV_HUGEPAGE);
#endif
 }
 u32* take(size_t n) {
  u32* p= base + used;
  used+= words(n);
  return p;
 }
};
template <bool PEEL, int ALG= 0> struct Solver {  // ALG 0: Hopcroft–Tarjan、1: 記録の形の Hopcroft–Tarjan、2: Tarjan–Vishkin
 struct F {
  u32 v, i, e;
 };
 int n;
 const vector<array<int, 2>>& es;
 Mem mem;
 u32 *bv, *bo, nb= 0;  // 成分 k の頂点は bv[bo[k]] から bv[bo[k + 1] - 1]
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(6 * Mem::words(N + 1) + Mem::words(2 * m) + Mem::words(3 * size_t(N)) + Mem::words(N + m + 2) + Mem::words(N + m + 2) + (PEEL ? 2 * Mem::words(N) : 0) + (ALG ? 8 * Mem::words(N + 1) : 0));
  u32 *ord= mem.take(N + 1), *low= mem.take(N + 1), *deg= mem.take(N + 1), *off= mem.take(N + 1), *adj= mem.take(2 * m), *st= mem.take(N + 1);
  F* fs= reinterpret_cast<F*>(mem.take(3 * size_t(N)));
  bv= mem.take(N + m + 2), bo= mem.take(N + m + 2);
  memset(ord, 0, N * sizeof(u32)), memset(deg, 0, (N + 1) * sizeof(u32));
  u32 nv= 0;  // bv に置いた頂点の数
  bo[0]= 0;
  auto close= [&] { bo[++nb]= nv; };
  if constexpr(PEEL) {
   u32 *xr= mem.take(N), *q= mem.take(N);
   memset(xr, 0, N * sizeof(u32));
   for(auto& e: es) ++deg[e[0]], ++deg[e[1]], xr[e[0]]^= e[1], xr[e[1]]^= e[0];
   // 辺を持たない頂点は 1 頂点の成分。次数 1 の頂点を積み、次数がちょうど 1 になったら積む。剥がした頂点の ord は印として NIL にする。
   u32 qt= 0;
   for(u32 v= 0; v < N; ++v)
    if(!deg[v]) ord[v]= ~u32(0), bv[nv++]= v, close();
    else q[qt]= v, qt+= deg[v] == 1;
   for(u32 qh= 0; qh < qt; ++qh) {
    const u32 v= q[qh];
    ord[v]= ~u32(0);
    if(!deg[v]) continue;
    const u32 u= xr[v];
    deg[v]= 0, xr[u]^= v, bv[nv++]= v, bv[nv++]= u, close();
    if(--deg[u] == 1) q[qt++]= u;
   }
  } else {
   for(auto& e: es) ++deg[e[0]], ++deg[e[1]];
  }
  // CSR。off[x] は x までの次数の累積で、辺を置くたびに減らして off[x] を x の始まりにする。剥がした頂点の次数は 0。
  off[0]= deg[0];
  for(u32 x= 1; x < N; ++x) off[x]= off[x - 1] + deg[x];
  off[N]= N ? off[N - 1] : 0;
  for(auto& e: es) {
   const u32 a= e[0], b= e[1];
   if constexpr(PEEL)
    if(ord[a] | ord[b]) continue;
   adj[--off[a]]= b, adj[--off[b]]= a;
  }
  if constexpr(ALG == 2) {
   tv(off, adj, ord, mem.take(N + 1), mem.take(N + 1), mem.take(N + 1), mem.take(N + 1), mem.take(N + 1), mem.take(N + 1), st, nv);
   return;
  }
  if constexpr(ALG == 1) {
   dfs_rec(off, adj, ord, st, reinterpret_cast<R*>(mem.take(4 * size_t(N))), reinterpret_cast<FR*>(fs), nv);
   return;
  }
  u32 idx= 0, top= 0;
  for(u32 r= 0; r < N; ++r) {
   if(ord[r]) continue;
   if(off[r] == off[r + 1]) {
    // 辺を持たない頂点 (PEEL なら剥がしで先に置いてある)
    ord[r]= ++idx, bv[nv++]= r, close();
    continue;
   }
   u32 v= r, i= off[r], e= off[r + 1];
   F* sp= fs;
   ord[v]= low[v]= ++idx, st[top++]= v;
   for(;;) {
    if(i != e) {
     const u32 w= adj[i++], ow= ord[w];
     if(!ow) *sp++= {v, i, e}, v= w, i= off[w], e= off[w + 1], ord[v]= low[v]= ++idx, st[top++]= v;
     else if(ow < low[v]) low[v]= ow;
    } else {
     if(sp == fs) {
      --top;  // 根。子の成分はどれも子から帰るときに切り出してあるので、根だけが残っている
      break;
     }
     const u32 lv= low[v], c= v;
     --sp, v= sp->v, i= sp->i, e= sp->e;
     if(lv >= ord[v]) {
      u32 x;
      do x= st[--top], bv[nv++]= x;
      while(x != c);
      bv[nv++]= v, close();
     } else if(lv < low[v]) low[v]= lv;
    }
   }
  }
 }
 struct R {
  u32 s, e, n0, n1;
 };
 struct FR {
  u32 v, i, lv;
 };
 // いまの頂点の隣接の位置 i が s なら n0、s + 1 なら n1、それより後ろは adj[i]。lv はいまの頂点の lowlink。
 void dfs_rec(const u32* off, const u32* adj, u32* ord, u32* st, R* rec, FR* fs, u32 nv) {
  const u32 N= n;
  auto close= [&] { bo[++nb]= nv; };
  for(u32 v= 0; v < N; ++v) {
   const u32 s= off[v], e= off[v + 1];
   rec[v]= {s, e, s < e ? adj[s] : ~u32(0), s + 1 < e ? adj[s + 1] : ~u32(0)};
  }
  u32 idx= 0, top= 0;
  for(u32 r= 0; r < N; ++r) {
   if(ord[r]) continue;
   R c= rec[r];
   if(c.s == c.e) {
    ord[r]= ++idx, bv[nv++]= r, close();
    continue;
   }
   u32 v= r, i= c.s, lv= ord[v]= ++idx;
   FR* sp= fs;
   st[top++]= v;
   for(;;) {
    if(i != c.e) {
     const u32 w= i == c.s ? c.n0 : i == c.s + 1 ? c.n1 : adj[i], ow= ord[w];
     ++i;
     if(!ow) *sp++= {v, i, lv}, v= w, c= rec[w], i= c.s, lv= ord[v]= ++idx, st[top++]= v;
     else if(ow < lv) lv= ow;
    } else {
     if(sp == fs) {
      --top;
      break;
     }
     const u32 lc= lv, ch= v;
     --sp, v= sp->v, i= sp->i, lv= sp->lv, c= rec[v];
     if(lc >= ord[v]) {
      u32 x;
      do x= st[--top], bv[nv++]= x;
      while(x != ch);
      bv[nv++]= v, close();
     } else if(lc < lv) lv= lc;
    }
   }
  }
 }
 // Tarjan–Vishkin。ord の 0 でない頂点 (剥がした頂点) は飛ばす。q は BFS の順、par は BFS の木の親 (根は自分)、bi は BFS の
 // 順 + 1 (0 は未訪問)。pre は初めに NIL で、木の子に番号を配ったら NIL でなくなる (多重辺で同じ子を 2 度数えない)。
 // d は union-find の親、ws は作業用 (成分ごとの数、置き場)。
 void tv(const u32* off, const u32* adj, const u32* ord, u32* q, u32* par, u32* nd, u32* pre, u32* lo, u32* hi, u32* d, u32 nv) {
  const u32 N= n, NIL= ~u32(0);
  auto close= [&] { bo[++nb]= nv; };
  u32* bi= d;  // BFS の間だけ d を訪問の印に使う
  memset(bi, 0, N * sizeof(u32));
  u32 qt= 0;
  for(u32 r= 0; r < N; ++r) {
   if(ord[r] || bi[r]) continue;
   if(off[r] == off[r + 1]) {
    // 辺を持たない頂点 (PEEL なら剥がしで先に置いてある)
    bi[r]= NIL, bv[nv++]= r, close();
    continue;
   }
   u32 qh= qt;
   q[qt++]= r, bi[r]= qt, par[r]= r;
   while(qh < qt) {
    const u32 v= q[qh++];
    for(u32 i= off[v], e= off[v + 1]; i < e; ++i) {
     const u32 w= adj[i];
     if(!bi[w]) q[qt++]= w, bi[w]= qt, par[w]= v;
    }
   }
  }
  // 部分木の大きさ (逆順)、行きがけ順の番号 (BFS の順に、子に区間を配る)。
  for(u32 k= 0; k < qt; ++k) nd[q[k]]= 1, pre[q[k]]= NIL;
  for(u32 k= qt; k--;) {
   const u32 v= q[k];
   if(par[v] != v) nd[par[v]]+= nd[v];
  }
  u32 base= 0;
  for(u32 k= 0; k < qt; ++k) {
   const u32 v= q[k];
   if(par[v] == v) pre[v]= base, base+= nd[v];
   u32 o= pre[v] + 1;
   for(u32 i= off[v], e= off[v + 1]; i < e; ++i) {
    const u32 w= adj[i];
    if(par[w] == v && pre[w] == NIL) pre[w]= o, o+= nd[w];
   }
  }
  // lo と hi。頂点の隣接の pre で初めにしてから、逆順に子から親へ畳む。
  for(u32 k= 0; k < qt; ++k) {
   const u32 v= q[k];
   u32 l= pre[v], h= l;
   for(u32 i= off[v], e= off[v + 1]; i < e; ++i) {
    const u32 x= pre[adj[i]];
    l= std::min(l, x), h= std::max(h, x);
   }
   lo[v]= l, hi[v]= h;
  }
  for(u32 k= qt; k--;) {
   const u32 v= q[k], p= par[v];
   if(p != v) lo[p]= std::min(lo[p], lo[v]), hi[p]= std::max(hi[p], hi[v]);
  }
  // union-find。木の辺 (par[v], v) を v で表す。
  for(u32 k= 0; k < qt; ++k) d[q[k]]= q[k];
  auto find= [&](u32 x) {
   while(d[x] != x) d[x]= d[d[x]], x= d[x];
   return x;
  };
  auto unite= [&](u32 a, u32 b) {
   a= find(a), b= find(b);
   if(a != b) d[a]= b;
  };
  auto anc= [&](u32 u, u32 w) { return pre[u] <= pre[w] && pre[w] < pre[u] + nd[u]; };
  for(auto& ed: es) {
   const u32 a= ed[0], b= ed[1];
   if constexpr(PEEL)
    if(ord[a] | ord[b]) continue;
   if(!anc(a, b) && !anc(b, a)) unite(a, b);
  }
  for(u32 k= 0; k < qt; ++k) {
   const u32 v= q[k], p= par[v];
   if(p == v || par[p] == p) continue;
   if(lo[v] < pre[p] || hi[v] >= pre[p] + nd[p]) unite(v, p);
  }
  // 成分ごとに頂点を集める。根でない頂点を組ごとに数えて置き場を配り、組の頂点を置いてから、組のいちばん上の頂点 (親が根か
  // 別の組) の親を末尾に足す。nd と lo と hi を作業に使い回す: nd[c] は組 c の頂点の数、lo[c] は組 c の次の置き場、hi[c] は組 c の
  // いちばん上の頂点の親。union-find の根 (d[c] == c) で、BFS の根でない頂点が組の代表。
  for(u32 k= 0; k < qt; ++k) nd[q[k]]= 0;
  for(u32 k= 0; k < qt; ++k) {
   const u32 v= q[k];
   if(par[v] != v) ++nd[find(v)];
  }
  for(u32 k= 0; k < qt; ++k) {
   const u32 c= q[k];
   if(par[c] != c && d[c] == c) lo[c]= nv, nv+= nd[c] + 1, close();
  }
  for(u32 k= 0; k < qt; ++k) {
   const u32 v= q[k], p= par[v];
   if(p == v) continue;
   const u32 c= find(v);
   bv[lo[c]++]= v;
   if(par[p] == p || find(p) != c) hi[c]= p;
  }
  for(u32 k= 0; k < qt; ++k) {
   const u32 c= q[k];
   if(par[c] != c && d[c] == c) bv[lo[c]]= hi[c];
  }
 }
 vector<vector<int>> answer() const {
  vector<vector<int>> ret(nb);
  for(u32 k= 0; k < nb; ++k) ret[k].assign(bv + bo[k], bv + bo[k + 1]);
  return ret;
 }
};
}
