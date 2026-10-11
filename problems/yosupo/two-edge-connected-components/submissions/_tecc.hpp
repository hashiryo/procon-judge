#pragma once
// 二辺連結成分の核。隣接は CSR で、自己ループは入れない (二辺連結性に関わらない)。DFS は反復で書き、親への木の辺は、多重辺を
// 正しく扱うため、隣接の中で親を初めて見たときだけ飛ばす (2 本目からの平行な辺は後ろ向きの辺として扱う)。
//
// DFS は path-based (Gabow)。鍵は st に載っている間はその位置 (1 から) で、bs は成分の根の候補の位置のスタック。
// 後ろ向きの辺 (v, w) では、w の位置より上の候補を捨てる。v から帰るとき、v の位置が候補のいちばん上なら、st の v から上が 1 つの
// 成分になる。無向グラフの DFS には横向きの辺が無く、成分に入った頂点への辺は子孫への辺なので、成分に入った頂点の鍵を
// どの位置より大きい値にしておけば、何も捨てない。
//
// PEEL なら、DFS の前に次数 1 以下の頂点を待ち行列で剥がす。剥がした頂点はどれも 1 頂点の成分 (その辺は橋)。頂点ごとに次数と、
// 残っている隣接の番号の XOR を持つので、次数が 1 になった頂点の残りの隣接は XOR の値そのもので、隣接を組まずに辺の列だけで
// 剥がせる。剥がしたあとの次数はそのまま 2-core の CSR の次数になるので、CSR は 2-core の辺だけで組む。
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
template <bool PEEL> struct Solver {
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
  mem.reserve(6 * Mem::words(N + 1) + Mem::words(2 * m) + Mem::words(4 * size_t(N)) + (PEEL ? 2 * Mem::words(N) : 0));
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
  gabow(off, adj, st, aux, fs);
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
 int comp(int v) const { return int(NIL - key[v]); }
};
}
