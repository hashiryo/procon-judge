#pragma once
// 二辺連結成分の核。隣接は CSR で、自己ループは入れない (二辺連結性に関わらない)。DFS は反復で書き、親への木の辺は、多重辺を
// 正しく扱うため、隣接の中で親を初めて見たときだけ飛ばす (2 本目からの平行な辺は後ろ向きの辺として扱う)。
//
// ALG = 0 は path-based (Gabow)。鍵は st に載っている間はその位置 (1 から) で、bs は成分の根の候補の位置のスタック。
// 後ろ向きの辺 (v, w) では、w の位置より上の候補を捨てる。v から帰るとき、v の位置が候補のいちばん上なら、st の v から上が 1 つの
// 成分になる。無向グラフの DFS には横向きの辺が無く、成分に入った頂点への辺は子孫への辺なので、成分に入った頂点の鍵を
// どの位置より大きい値にしておけば、何も捨てない。
// ALG = 1 は lowlink (Tarjan)。訪問順 ord と lowlink の low を頂点ごとの配列で持ち、low[v] == ord[v] の頂点から帰るとき、st の
// v から上を 1 つの成分にする。成分に入った頂点への辺は子孫への辺で ord が大きいので、low を下げない。
//
// PEEL なら、DFS の前に次数 1 以下の頂点を待ち行列で剥がす。剥がした頂点はどれも 1 頂点の成分 (その辺は橋)。頂点ごとに次数と、
// 残っている隣接の番号の XOR を持つので、次数が 1 になった頂点の残りの隣接は XOR の値そのもので、隣接を組まずに辺の列だけで
// 剥がせる。剥がしたあとの次数はそのまま 2-core の CSR の次数になるので、CSR は 2-core の辺だけで組む。
//
// ALG = 2 は DFS を使わない。BFS で全域森を作り、木に入らない辺ごとに、両端から BFS の順が後ろの側を親へ上げながら union-find で
// つなぐ (木の辺はどれも高々 1 回しかつながないので、全体で O(M α))。BFS は待ち行列の頂点ごとに読み込みが独立で、DFS のように
// 読み込みが 1 本の鎖につながらない。どの辺を木の辺とみなすかは、頂点 b の親が a の辺 (a, b) のうち、辺の列で最初のもの (多重辺の
// 残りは木に入らない辺)。成分の名前は union-find の根で、つないだ道のいちばん上の頂点。
// ALG = 3 は ALG = 0 の DFS を、頂点ごとの記録 {CSR の始まり, 終わり, 最初の隣接, 2 番目の隣接} (16 byte) で回す。無向グラフの
// 隣接の 1 つは親なので、最初の隣接だけを置くと半分の頂点で親に当たる。2 つ置けば、次数 2 の頂点 (閉路の頂点) では DFS が下りる
// 1 歩が記録の読み込み 1 段で済む。フレームには頂点と位置と親だけを持ち、戻るときに記録を読み直す。
//
// 成分の名前は、DFS で成分を閉じた頂点か、剥がした頂点そのもの。鍵は成分に入ったら NIL - (名前) にする。作業領域は 2 MB 境界の
// 1 本の領域で、huge page を頼む (yosupo-scc の _scc.hpp の Mem<true> と同じ)。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace tecc {
constexpr u32 NIL= ~u32(0);
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
template <bool PEEL, int ALG> struct Solver {
 struct F {
  u32 v, i, e, par;
 };
 int n;
 const vector<array<int, 2>>& es;
 Mem mem;
 u32* key;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(14 * Mem::words(N + 1) + Mem::words(2 * m) + Mem::words(4 * size_t(N)) + (PEEL ? 2 * Mem::words(N) : 0));
  key= mem.take(N);
  u32 *deg= mem.take(N + 1), *off= mem.take(N + 1), *adj= mem.take(2 * m), *st= mem.take(N + 1), *aux= mem.take(N + 1);
  F* fs= reinterpret_cast<F*>(mem.take(4 * size_t(N)));
  memset(key, 0, N * sizeof(u32)), memset(deg, 0, (N + 1) * sizeof(u32));
  if constexpr(PEEL) {
   u32 *xr= mem.take(N), *q= mem.take(N);
   memset(xr, 0, N * sizeof(u32));
   for(auto& e: es)
    if(e[0] != e[1]) ++deg[e[0]], ++deg[e[1]], xr[e[0]]^= e[1], xr[e[1]]^= e[0];
   // 次数 1 以下で積み、次数がちょうど 1 になったら積む。どの頂点も高々 1 回しか積まれない。
   u32 qt= 0;
   for(u32 v= 0; v < N; ++v) q[qt]= v, qt+= deg[v] <= 1;
   for(u32 qh= 0; qh < qt; ++qh) {
    const u32 v= q[qh];
    key[v]= NIL - v;
    if(deg[v]) {
     const u32 u= xr[v];
     deg[v]= 0, xr[u]^= v;
     if(--deg[u] == 1) q[qt++]= u;
    }
   }
  } else {
   for(auto& e: es)
    if(e[0] != e[1]) ++deg[e[0]], ++deg[e[1]];
  }
  // CSR。off[x] は x までの次数の累積で、辺を置くたびに減らして off[x] を x の始まりにする。剥がした頂点の次数は 0。
  off[0]= deg[0];
  for(u32 x= 1; x < N; ++x) off[x]= off[x - 1] + deg[x];
  off[N]= N ? off[N - 1] : 0;
  for(auto& e: es) {
   const u32 a= e[0], b= e[1];
   if(a == b) continue;
   if constexpr(PEEL)
    if(key[a] | key[b]) continue;
   adj[--off[a]]= b, adj[--off[b]]= a;
  }
  if constexpr(ALG == 0) gabow(off, adj, st, aux, fs);
  else if constexpr(ALG == 1) tarjan(off, adj, st, aux, fs);
  else if constexpr(ALG == 2) bfs_dsu(off, adj, mem.take(N + 1), mem.take(N + 1), mem.take(N + 1), mem.take(N + 1), mem.take(N + 1));
  else gabow_rec(off, adj, st, aux, reinterpret_cast<R*>(mem.take(4 * size_t(N))), reinterpret_cast<F3*>(fs));
 }
 struct R {
  u32 s, e, n0, n1;
 };
 struct F3 {
  u32 v, i, par;
 };
 // bi[v] は BFS の順 (1 から、0 は未訪問)、par[v] は BFS の木の親 (根は自分)、d[v] は union-find の親、used[v] は v の木の辺を
 // 辺の列でもう見たか。
 void bfs_dsu(const u32* off, const u32* adj, u32* q, u32* par, u32* bi, u32* d, u32* used) {
  const u32 N= n;
  memset(bi, 0, N * sizeof(u32)), memset(used, 0, N * sizeof(u32));
  u32 qt= 0;
  for(u32 r= 0; r < N; ++r) {
   if(key[r] || bi[r]) continue;
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
  for(u32 v= 0; v < N; ++v) d[v]= v;
  auto find= [&](u32 x) {
   while(d[x] != x) d[x]= d[d[x]], x= d[x];
   return x;
  };
  for(auto& ed: es) {
   const u32 a= ed[0], b= ed[1];
   if(a == b) continue;
   if constexpr(PEEL)
    if(key[a] | key[b]) continue;
   if(par[b] == a && !used[b]) {
    used[b]= 1;
    continue;
   }
   if(par[a] == b && !used[a]) {
    used[a]= 1;
    continue;
   }
   u32 u= find(a), w= find(b);
   while(u != w) {
    if(bi[u] < bi[w]) std::swap(u, w);
    d[u]= par[u], u= find(u);
   }
  }
  for(u32 v= 0; v < N; ++v)
   if(!key[v]) key[v]= NIL - find(v);
 }
 // ALG = 0 と同じ path-based を、記録の形で回す。いまの頂点の隣接の位置 i が s なら n0、s + 1 なら n1、それより後ろは adj[i]。
 void gabow_rec(const u32* off, const u32* adj, u32* st, u32* bs, R* rec, F3* fs) {
  const u32 N= n;
  for(u32 v= 0; v < N; ++v) {
   const u32 s= off[v], e= off[v + 1];
   rec[v]= {s, e, s < e ? adj[s] : NIL, s + 1 < e ? adj[s + 1] : NIL};
  }
  u32 top= 0, bt= 0;
  for(u32 r= 0; r < N; ++r) {
   if(key[r]) continue;
   R c= rec[r];
   u32 v= r, i= c.s, par= NIL;
   F3* sp= fs;
   st[top++]= v, key[v]= top, bs[bt++]= top;
   for(;;) {
    if(i != c.e) {
     const u32 w= i == c.s ? c.n0 : i == c.s + 1 ? c.n1 : adj[i];
     ++i;
     if(w == par) {
      par= NIL;
      continue;
     }
     const u32 kw= key[w];
     if(!kw) *sp++= {v, i, par}, par= v, v= w, c= rec[w], i= c.s, st[top++]= v, key[v]= top, bs[bt++]= top;
     else
      while(kw < bs[bt - 1]) --bt;
    } else {
     const u32 p= key[v];
     if(p == bs[bt - 1]) {
      --bt;
      while(top >= p) key[st[--top]]= NIL - v;
     }
     if(sp == fs) break;
     --sp, v= sp->v, i= sp->i, par= sp->par, c= rec[v];
    }
   }
  }
 }
 void gabow(const u32* off, const u32* adj, u32* st, u32* bs, F* fs) {
  u32 top= 0, bt= 0;
  for(u32 r= 0; r < u32(n); ++r) {
   if(key[r]) continue;
   u32 v= r, i= off[r], e= off[r + 1], par= NIL;
   F* sp= fs;
   st[top++]= v, key[v]= top, bs[bt++]= top;
   for(;;) {
    if(i != e) {
     const u32 w= adj[i++];
     if(w == par) {
      par= NIL;
      continue;
     }
     const u32 kw= key[w];
     if(!kw) *sp++= {v, i, e, par}, par= v, v= w, i= off[w], e= off[w + 1], st[top++]= v, key[v]= top, bs[bt++]= top;
     else
      while(kw < bs[bt - 1]) --bt;
    } else {
     const u32 p= key[v];
     if(p == bs[bt - 1]) {
      --bt;
      while(top >= p) key[st[--top]]= NIL - v;
     }
     if(sp == fs) break;
     --sp, v= sp->v, i= sp->i, e= sp->e, par= sp->par;
    }
   }
  }
 }
 // ord は key に置き、low は aux に置く。成分に入った頂点の key は NIL - (名前) で、どの ord より大きい。
 void tarjan(const u32* off, const u32* adj, u32* st, u32* low, F* fs) {
  u32 idx= 0, top= 0;
  for(u32 r= 0; r < u32(n); ++r) {
   if(key[r]) continue;
   u32 v= r, i= off[r], e= off[r + 1], par= NIL;
   F* sp= fs;
   key[v]= low[v]= ++idx, st[top++]= v;
   for(;;) {
    if(i != e) {
     const u32 w= adj[i++];
     if(w == par) {
      par= NIL;
      continue;
     }
     const u32 kw= key[w];
     if(!kw) *sp++= {v, i, e, par}, par= v, v= w, i= off[w], e= off[w + 1], key[v]= low[v]= ++idx, st[top++]= v;
     else if(kw < low[v]) low[v]= kw;
    } else {
     const u32 lv= low[v];
     if(lv == key[v]) {
      u32 x;
      do x= st[--top], key[x]= NIL - v;
      while(x != v);
     }
     if(sp == fs) break;
     --sp, v= sp->v, i= sp->i, e= sp->e, par= sp->par;
     if(lv < low[v]) low[v]= lv;
    }
   }
  }
 }
 int comp(int v) const { return int(NIL - key[v]); }
};
}
