#pragma once
// 三辺連結成分の核。tsin.hpp と同じ Tsin の吸収の形の DFS を、2 MB 境界の 1 本の作業領域 (huge page を頼む) の u32 の配列で回す。
// 自己ループは入れず、親への木の辺は 1 本だけ飛ばす。訪問の印は行きがけ順の番号 in で兼ね (NIL なら未訪問)、low の無限大も NIL。
// deg は符号なしで持つ (参照解の size_t と同じく、負になるときは大きい値になり、帰るときの deg <= 1 と deg == 0 の判定は変わらない)。
//
// PEEL なら、DFS の前に次数 1 以下の頂点を、次数と残っている隣接の XOR だけで分岐なしに剥がす (_tecc2.hpp と同じ)。剥がした頂点の
// 辺は橋なので、剥がした頂点はどれも 1 頂点の成分。剥がしたあとの 2-core の頂点に 0 から番号を振り直し、2-core の辺だけで CSR を
// 組む (端が剥がれた辺は、辺の番号で回す 8 つの番兵に書いて捨てる)。次数と XOR は 1 つの配列に並べ、辺の端を先読みする。
//
// 吸収した頂点は、吸収した祖先を親に持つ。成分の名前は、行きがけ順に親の名前を写して決める (吸収する祖先は吸収される頂点より先に
// 訪れている)。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace t3 {
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
  u32 v, p, i;
 };
 int n;
 const vector<array<int, 2>>& es;
 Mem mem;
 u32 *id= nullptr, *orig= nullptr, *lab;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(12 * Mem::words(N + 10) + Mem::words(2 * m + 2) + Mem::words(3 * size_t(N)));
  u32 *dg= mem.take(2 * size_t(N + 1)), *q= mem.take(N + 1), *off= mem.take(N + 10), *adj= mem.take(2 * m + 2);
  u32 n2= N;
  if constexpr(PEEL) {
   id= mem.take(N + 1);
   // dg[2 v] が次数、dg[2 v + 1] が残っている隣接の XOR。
   memset(dg, 0, 2 * size_t(N) * sizeof(u32));
   for(size_t i= 0; i < m; ++i) {
    const auto& f= es[std::min(i + 16, m - 1)];
    __builtin_prefetch(dg + 2 * f[0]), __builtin_prefetch(dg + 2 * f[1]);
    const u32 a= es[i][0], b= es[i][1], d= a != b;
    dg[2 * a]+= d, dg[2 * b]+= d, dg[2 * a + 1]^= b & -d, dg[2 * b + 1]^= a & -d;
   }
   u32 qt= 0;
   for(u32 v= 0; v < N; ++v) q[qt]= v, qt+= dg[2 * v] == 1;
   for(u32 qh= 0; qh < qt; ++qh) {
    __builtin_prefetch(dg + 2 * q[std::min(qh + 16, qt - 1)]);
    __builtin_prefetch(dg + 2 * dg[2 * q[std::min(qh + 8, qt - 1)] + 1]);
    const u32 v= q[qh], live= dg[2 * v], u= dg[2 * v + 1];
    dg[2 * v]= 0, dg[2 * u + 1]^= v & -live;
    const u32 du= dg[2 * u] - live;
    dg[2 * u]= du, q[qt]= u, qt+= live & (du == 1);
   }
   orig= q;
   u32 s= 0;
   n2= 0;
   for(u32 v= 0; v < N; ++v) {
    const u32 d= dg[2 * v], c= -u32(d >= 2);
    id[v]= n2 | ~c, orig[n2]= v, s+= d, off[n2]= s, n2-= c;
   }
   for(u32 j= 0; j <= 8; ++j) off[n2 + j]= s;
   for(size_t i= 0; i < m; ++i) {
    const auto& f= es[std::min(i + 16, m - 1)];
    __builtin_prefetch(id + f[0]), __builtin_prefetch(id + f[1]);
    const auto& g= es[std::min(i + 8, m - 1)];
    __builtin_prefetch(off + std::min(id[g[0]], n2)), __builtin_prefetch(off + std::min(id[g[1]], n2));
    const u32 ia= id[es[i][0]], ib= id[es[i][1]], sk= -u32((ia == NIL) | (ib == NIL) | (ia == ib)), dm= n2 + 1 + (u32(i) & 7);
    const u32 a= (ia & ~sk) | (dm & sk), b= (ib & ~sk) | (dm & sk), l= 1 + sk;
    const u32 pa= off[a] - l;
    adj[pa]= b, off[a]= pa;
    const u32 pb= off[b] - l;
    adj[pb]= a, off[b]= pb;
   }
  } else {
   // CSR。off[x] は x までの次数の累積で、辺を置くたびに減らして始まりにする。自己ループは数えない。
   u32* d= dg;
   memset(d, 0, N * sizeof(u32));
   for(auto& e: es) {
    const u32 a= e[0], b= e[1], c= a != b;
    d[a]+= c, d[b]+= c;
   }
   u32 s= 0;
   for(u32 v= 0; v < N; ++v) s+= d[v], off[v]= s;
   off[N]= s;
   for(auto& e: es) {
    const u32 a= e[0], b= e[1];
    if(a != b) adj[--off[a]]= b, adj[--off[b]]= a;
   }
  }
  u32 *in= mem.take(N + 1), *out= mem.take(N + 1), *low= mem.take(N + 1), *deg= mem.take(N + 1), *path= mem.take(N + 1), *par= mem.take(N + 1), *ord= mem.take(N + 1);
  F* fs= reinterpret_cast<F*>(mem.take(3 * size_t(N)));
  memset(in, 0xff, n2 * sizeof(u32));
  u32 cnt= 0;
  for(u32 r= 0; r < n2; ++r) {
   if(in[r] != NIL) continue;
   u32 v= r, p= NIL, i= off[r];
   F* sp= fs;
   in[v]= cnt, ord[cnt++]= v, low[v]= NIL, deg[v]= 0, path[v]= NIL, par[v]= v;
   for(;;) {
    if(i != off[v + 1]) {
     const u32 w= adj[i++];
     if(w == p) {
      p= NIL;
      continue;
     }
     const u32 iw= in[w];
     if(iw != NIL) {
      if(iw < in[v]) ++deg[v], low[v]= std::min(low[v], iw);
      else {
       --deg[v];
       u32 u= path[v];
       while(u != NIL && in[u] <= iw && iw < out[u]) par[u]= v, deg[v]+= deg[u], u= path[u];
       path[v]= u;
      }
      continue;
     }
     *sp++= {v, p, i}, p= v, v= w, i= off[w];
     in[v]= cnt, ord[cnt++]= v, low[v]= NIL, deg[v]= 0, path[v]= NIL, par[v]= v;
    } else {
     out[v]= cnt;
     if(sp == fs) break;
     u32 w= v;
     --sp, v= sp->v, p= sp->p, i= sp->i;
     if(path[w] == NIL && deg[w] <= 1) {
      deg[v]+= deg[w], low[v]= std::min(low[v], low[w]);
      continue;
     }
     if(deg[w] == 0) w= path[w];
     if(low[w] < low[v]) low[v]= low[w], std::swap(w, path[v]);
     while(w != NIL) par[w]= v, deg[v]+= deg[w], w= path[w];
    }
   }
  }
  lab= low;  // low は使い終わった
  for(u32 t= 0; t < cnt; ++t) {
   const u32 x= ord[t], y= par[x];
   lab[x]= y == x ? x : lab[y];
  }
 }
 int comp(int v) const {
  if constexpr(PEEL) {
   const u32 x= id[v];
   return int(x == NIL ? u32(v) : orig[lab[x]]);
  } else {
   return int(lab[v]);
  }
 }
};
}
