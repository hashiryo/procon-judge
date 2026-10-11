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
template <bool PEEL> struct Solver {
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
  mem.reserve(6 * Mem::words(N + 1) + Mem::words(2 * m) + Mem::words(3 * size_t(N)) + Mem::words(N + m + 2) + Mem::words(N + m + 2) + (PEEL ? 2 * Mem::words(N) : 0));
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
 vector<vector<int>> answer() const {
  vector<vector<int>> ret(nb);
  for(u32 k= 0; k < nb; ++k) ret[k].assign(bv + bo[k], bv + bo[k + 1]);
  return ret;
 }
};
}
