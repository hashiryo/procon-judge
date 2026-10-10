#pragma once
// 2-SAT に合わせた核。含意グラフの辺の列も、SCC の記録の形も作らず、リテラルの出現の CSR を 1 つだけ組む。リテラル x の出現の
// 並び occ[x] は、x を含む節のもう一方のリテラルの並びで、含意グラフでは次のようになる。
//   x から出る辺の行き先 = occ[¬x] (節 ¬x ∨ z から辺 x → z)
//   x の入次数 = |occ[x]| (節 x ∨ z から辺 ¬z → x)
// 節を 1 回なめて出現を数えれば、CSR の区切りと、剥がしに使う入次数の両方が手に入る (数えた配列をそのまま入次数にする)。
// 頂点 (リテラル) ごとの配列は鍵の 4 byte だけで、SCC の記録の形 (8 byte か 16 byte) より小さい。max_random の含意グラフは
// 10^6 頂点あり、yosupo-scc の形では作業領域が L3 から溢れていたので、小さく持つ。
// 剥がしと DFS (path-based) の手順は yosupo-scc の _scc_rec.hpp の GabowRec と同じで、頂点 v の出辺の範囲が
// off[v ^ 1] から off[(v ^ 1) + 1] になるところだけが違う。PF なら剥がしで待ち行列の先の頂点の範囲と出現を先読みする。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace occ {
constexpr u32 NIL= ~u32(0);
// 作業領域。2 MB 境界の 1 本の領域に取り、huge page を頼む (yosupo-scc の _scc.hpp の Mem<true> と同じ)。
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
// リテラル l (負なら否定) を頂点にする。下位 1 bit が否定。
inline u32 vertex(int l) { return 2 * u32(l < 0 ? -l : l) - 2 + (l < 0); }
template <bool PF> struct Solver {
 struct F {
  u32 v, i, e;
 };
 int n;
 const vector<array<int, 2>>& cs;
 Mem mem;
 u32 N, P= 0, base= 0;
 u32 *key, *off, *adj;
 bool sat= true;
 Solver(int n, const vector<array<int, 2>>& cs): n(n), cs(cs) {}
 void run() {
  N= 2 * n;
  const size_t m= cs.size();
  mem.reserve(5 * Mem::words(N + 1) + Mem::words(N + 33) + Mem::words(2 * m + 1) + Mem::words(N * sizeof(F) / sizeof(u32)));
  u32* deg= mem.take(N + 1);                                         // 出現の数 = 入次数。deg[N] は剥がしの番兵
  off= mem.take(N + 1), adj= mem.take(2 * m + 1), key= mem.take(N);  // adj は先読みのために 1 語多く取る
  u32 *q= mem.take(N + 33), *st= mem.take(N + 1), *bs= mem.take(N + 1);
  F* fs= reinterpret_cast<F*>(mem.take(N * sizeof(F) / sizeof(u32)));
  memset(deg, 0, (N + 1) * sizeof(u32));
  for(auto& c: cs) ++deg[vertex(c[0])], ++deg[vertex(c[1])];
  // off[x] は x までの出現の数の累積。出現を後ろから置いて off[x] を x の始まりにする。
  off[0]= deg[0];
  for(u32 x= 1; x < N; ++x) off[x]= off[x - 1] + deg[x];
  off[N]= 2 * m;
  for(size_t j= m; j--;) {
   const u32 a= vertex(cs[j][0]), b= vertex(cs[j][1]);
   adj[--off[a]]= b, adj[--off[b]]= a;
  }
  memset(key, 0, N * sizeof(u32));
  peel(deg, q);
  dfs(st, bs, fs);
  for(int i= 0; i < n; ++i)
   if(key[2 * i] == key[2 * i + 1]) {
    sat= false;
    break;
   }
 }
 // 入次数 0 の頂点を剥がす。剥がした頂点の鍵は NIL - (剥がした順)。x を剥がすと、x の出辺の行き先 occ[x ^ 1] の入次数が減る。
 // 積むかどうかは分岐にせず、毎回 q の末尾に書いて、入次数が 0 になったときだけ末尾を進める。
 void peel(u32* deg, u32* q) {
  u32 qt= 0;
  for(u32 x= 0; x < N; ++x) q[qt]= x, qt+= deg[x] == 0;
  for(u32 qh= 0; qh < qt; ++qh) {
   if constexpr(PF) {
    // 16 個先の頂点の出辺の範囲、8 個先の頂点の出辺の行き先、4 個先の頂点の最初の行き先の入次数を先読みする。先読みの
    // 位置は待ち行列の末尾で止める。出辺が無い頂点の adj[off] は隣の頂点の行き先か範囲の外の 1 語なので、値を N で止める。
    __builtin_prefetch(off + (q[min(qh + 16, qt - 1)] ^ 1));
    __builtin_prefetch(adj + off[q[min(qh + 8, qt - 1)] ^ 1]);
    __builtin_prefetch(deg + min(adj[off[q[min(qh + 4, qt - 1)] ^ 1]], N));
   }
   const u32 x= q[qh];
   key[x]= NIL - qh;
   for(u32 i= off[x ^ 1], e= off[(x ^ 1) + 1]; i < e; ++i) {
    const u32 z= adj[i];
    q[qt]= z, qt+= --deg[z] == 0;
   }
  }
  P= qt;
 }
 // path-based。鍵は st に載っている間はその位置 (1 から)、成分に入ったら成分の番号 (2N から下がる)。
 void dfs(u32* st, u32* bs, F* fs) {
  u32 top= 0, bt= 0, cc= 2 * N;
  for(u32 r= 0; r < N; ++r) {
   if(key[r]) continue;
   u32 v= r, i= off[r ^ 1], e= off[(r ^ 1) + 1];
   F* sp= fs;
   st[top++]= v, key[v]= top, bs[bt++]= top;
   for(;;) {
    if(i != e) {
     const u32 w= adj[i++], kw= key[w];
     if(!kw) *sp++= {v, i, e}, v= w, i= off[w ^ 1], e= off[(w ^ 1) + 1], st[top++]= v, key[v]= top, bs[bt++]= top;
     else
      while(kw < bs[bt - 1]) --bt;
    } else {
     const u32 p= key[v];
     if(p == bs[bt - 1]) {
      --bt;
      while(top >= p) key[st[--top]]= cc;
      --cc;
     }
     if(sp == fs) break;
     --sp, v= sp->v, i= sp->i, e= sp->e;
    }
   }
  }
  base= cc + 1;
 }
 // 成分の番号 (トポロジカル順)。剥がした頂点は剥がした順、残りは剥がした数のあとに DFS の成分の番号を並べる。
 u32 comp(u32 v) const {
  const u32 x= key[v];
  return x > 2 * N ? NIL - x : x - base + P;
 }
 bool satisfiable() const { return sat; }
 bool value(int i) const { return comp(2 * i - 2) > comp(2 * i - 1); }
};
}
