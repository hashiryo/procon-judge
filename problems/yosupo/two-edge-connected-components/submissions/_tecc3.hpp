#pragma once
// 二辺連結成分の核の 3 本目。2-core の次数 2 の頂点が続く鎖を、DFS の前に何本も同時に歩いて縮め、残った頂点 (核) だけを DFS で
// 分ける。DFS は 1 歩ごとに読み込みを待つ鎖になるが、別々の鎖を歩く読み込みは独立なので、何本も交互に進めれば並べて出せる。
//
// 剥がしまでは _tecc2.hpp と同じ (分岐なし、先読みあり)。剥がしたあと、次数 3 以上の頂点と、次数 2 の頂点のうち番号のハッシュで
// 選んだ 2^-LOG_S ほどの頂点を核にする。残りの次数 2 の頂点 (鎖の頂点) は、核の頂点の間をつなぐ道 (鎖) をなすか、核を含まない
// 閉路をなす。頂点ごとの語 W[v] に、核なら KER | 核の番号、鎖の頂点なら残っている 2 つの隣接の XOR、剥がした頂点なら PEELED を
// 置く。次数 2 の頂点を選んで核に入れるのは、長い鎖や大きい閉路を短い鎖に切って、同時に歩く鎖の数を確保するため。
//
// 辺の列をもう一度なめて、核の頂点の CSR を組む。隣接が核ならその番号、鎖の頂点なら CHAIN | 頂点。鎖の頂点どうしの辺は捨てる。
// 書くかどうかは分岐にせず、書かない側は番兵の頂点 (核の番号 nk) の、減らさない位置に書く。
//
// 鎖は、核の CSR の CHAIN の欄ごとに、核の頂点から歩き出す (歩きの番号は CHAIN の欄の通し番号 j)。LANES 本を交互に 1 歩ずつ進め、
// 訪れた鎖の頂点の語を VIS | j にする。核の頂点に着くか、ほかの歩きが訪れた頂点に当たったら止め、欄に止まった頂点の語を書く。1 本の鎖は
// 両端から歩かれ、途中で出会うか、片方が端まで歩いて、もう片方は 1 歩目でもう訪れた頂点に当たる。どちらでも、欄 j の行き先は、
// 核に着いたならその頂点、出会ったなら相手の歩きが出た核の頂点になる。核を 1 つも含まない閉路は歩かれないので、訪れた頂点の数が
// 鎖の頂点の数に足りなければ、辺の列から隣接を 1 つずつ拾って、その閉路だけを 1 本ずつ歩く。
//
// 行き先を書いた核の CSR は、鎖を 1 本の辺に縮めた多重グラフ (鎖が輪になれば自己ループ) で、二辺連結成分は鎖を縮めても変わらない。
// これを _tecc2.hpp の記録の形の path-based で分ける。鎖の頂点は、鎖の両端の核の頂点が同じ成分ならその成分に入り、違えば鎖の
// 辺はどれも橋で、頂点ごとに 1 頂点の成分になる。歩きごとの成分の名前 (橋なら NIL) を chlab に置き、comp() は頂点の語から引く。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace tecc3 {
constexpr u32 NIL= ~u32(0);
// W の上の 3 bit。0 なら、まだ歩いていない鎖の頂点。
constexpr u32 KER= 0xA0000000u, VIS= 0xC0000000u, PEELED= 0xE0000000u, MASK= 0x1fffffffu, CHAIN= 0x80000000u;
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
template <int LOG_S= 5, int LANES= 16, bool D8= false> struct Solver {
 struct R {
  u32 s, e, n0, n1;
 };
 struct FG {
  u32 v, i, e, par, pv;
 };
 struct Lane {
  u32 j, prev, cur;
 };
 int n;
 const vector<array<int, 2>>& es;
 Mem mem;
 u32 *W, *korig, *key, *chlab;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(9 * Mem::words(N + 2) + 6 * Mem::words(2 * m + N + 2) + Mem::words(9 * size_t(N)));
  u32 *deg= mem.take(N + 1), *xr= mem.take(N + 1), *q= mem.take(N + 1);
  W= mem.take(N + 1), korig= mem.take(N + 1);
  u32 *koff= mem.take(N + 10), *kadj= mem.take(2 * m + 2), *sj= mem.take(2 * m + 2), *so= mem.take(2 * m + 2), *sf= mem.take(2 * m + 2), *sr= mem.take(2 * m + 2);
  chlab= mem.take(2 * m + N + 2);
  memset(deg, 0, N * sizeof(u32)), memset(xr, 0, N * sizeof(u32));
  for(auto& e: es) {
   const u32 a= e[0], b= e[1], d= a != b;
   deg[a]+= d, deg[b]+= d, xr[a]^= b & -d, xr[b]^= a & -d;
  }
  u32 qt= 0;
  for(u32 v= 0; v < N; ++v) q[qt]= v, qt+= deg[v] == 1;
  for(u32 qh= 0; qh < qt; ++qh) {
   const u32 a= q[std::min(qh + 16, qt - 1)];
   __builtin_prefetch(xr + a), __builtin_prefetch(deg + a);
   const u32 ub= xr[q[std::min(qh + 8, qt - 1)]];
   __builtin_prefetch(xr + ub), __builtin_prefetch(deg + ub);
   const u32 v= q[qh], live= deg[v], u= xr[v];
   deg[v]= 0, xr[u]^= v & -live;
   const u32 du= deg[u] - live;
   deg[u]= du, q[qt]= u, qt+= live & (du == 1);
  }
  // 核と鎖の頂点に分ける。koff[k] は核の頂点 k までの次数の累積 (CSR を組むときに減らして始まりにする)。
  u32 nk= 0, tot= 0, nc= 0;
  for(u32 v= 0; v < N; ++v) {
   const u32 d= deg[v], r= ((v * 0x9E3779B1u) >> (32 - LOG_S)) == 0, mk= -u32((d >= 3) | ((d == 2) & r)), mc= -u32((d == 2) & !r);
   W[v]= ((KER | nk) & mk) | (xr[v] & mc) | (PEELED & ~(mk | mc));
   korig[nk]= v, tot+= d & mk, koff[nk]= tot, nk-= mk, nc-= mc;
  }
  koff[nk]= tot;  // 番兵の頂点 nk (D8 なら nk + 1 から nk + 8 を辺の番号で回す) の書き込み位置 (減らさない)
  for(u32 j= 1; j <= 8; ++j) koff[nk + j]= tot;
  for(size_t i= 0; i < m; ++i) {
   const u32 a= es[i][0], b= es[i][1], wa= W[a], wb= W[b], dm= D8 ? nk + 1 + (u32(i) & 7) : nk;
   const u32 live= u32((wa >> 29) != 7) & u32((wb >> 29) != 7) & u32(a != b), ka= u32((wa >> 29) == 5), kb= u32((wb >> 29) == 5);
   const u32 da= ka & live, db= kb & live, mka= -ka, mkb= -kb, mda= -da, mdb= -db;
   const u32 va= ((wb & MASK) & mkb) | ((CHAIN | b) & ~mkb), vb= ((wa & MASK) & mka) | ((CHAIN | a) & ~mka);
   const u32 ta= ((wa & MASK) & mda) | (dm & ~mda), tb= ((wb & MASK) & mdb) | (dm & ~mdb);
   const u32 pa= koff[ta] - da;
   kadj[pa]= va, koff[ta]= pa;
   const u32 pb= koff[tb] - db;
   kadj[pb]= vb, koff[tb]= pb;
  }
  // 鎖の欄の通し番号 k ごとに、欄の位置 sj[k]、持ち主 so[k]、最初の鎖の頂点 sf[k] を並べる。歩きの番号は k。
  u32 ns= 0;
  for(u32 k= 0; k < nk; ++k)
   for(u32 j= koff[k], e= koff[k + 1]; j < e; ++j) sj[ns]= j, so[ns]= k, sf[ns]= kadj[j] & MASK, ns+= kadj[j] >> 31;
  // LANES 本を交互に進める。終わった歩きの sr[k] には、止まった頂点の語 (KER | 核の番号 か VIS | 出会った歩き) を書く。
  Lane L[LANES];
  u32 nl= 0, si= 0, nvis= 0;
  while(nl < LANES && si < ns) L[nl]= {si, korig[so[si]], sf[si]}, ++nl, ++si;
  while(nl) {
   for(u32 l= 0; l < nl;) {
    Lane& x= L[l];
    const u32 w= W[x.cur];
    if(w >> 31) {
     sr[x.j]= w;
     if(si < ns) x= {si, korig[so[si]], sf[si]}, ++si, ++l;
     else x= L[--nl];
    } else {
     W[x.cur]= VIS | x.j, ++nvis;
     const u32 nx= w ^ x.prev;
     x.prev= x.cur, x.cur= nx, ++l;
    }
   }
  }
  // 核を含まない閉路。剥がしていない隣接を 1 つずつ拾い (q を使い回す)、閉路ごとに歩く。歩きの番号は鎖の欄の数 ns から振り、名前は歩き出した頂点。
  u32 nj= ns;
  if(nvis != nc) {
   for(auto& e: es) {
    const u32 a= e[0], b= e[1];
    if(a == b || (W[a] >> 29) == 7 || (W[b] >> 29) == 7) continue;
    if(!(W[a] >> 31)) q[a]= b;
    if(!(W[b] >> 31)) q[b]= a;
   }
   for(u32 v= 0; v < N; ++v) {
    if(W[v] >> 31) continue;
    const u32 J= nj++;
    chlab[J]= v, W[v]= VIS | J;
    for(u32 prev= v, cur= q[v]; !(W[cur] >> 31);) {
     const u32 w= W[cur];
     W[cur]= VIS | J;
     const u32 nx= w ^ prev;
     prev= cur, cur= nx;
    }
   }
  }
  // 鎖の欄の行き先を核の番号にする。
  for(u32 k= 0; k < ns; ++k) {
   const u32 w= sr[k];
   kadj[sj[k]]= (w >> 29) == 5 ? w & MASK : so[w & MASK];
  }
  R* rec= reinterpret_cast<R*>(mem.take(4 * size_t(N)));
  FG* fs= reinterpret_cast<FG*>(mem.take(5 * size_t(N)));
  key= mem.take(N + 1);
  u32 *st= mem.take(N + 1), *bs= mem.take(N + 1);
  for(u32 k= 0; k < nk; ++k) {
   const u32 s= koff[k], e= koff[k + 1];
   rec[k]= {s, e, kadj[s], kadj[s + 1]};
  }
  memset(key, 0, nk * sizeof(u32));
  gabow_rec(nk, kadj, st, bs, rec, fs);
  for(u32 k= 0; k < ns; ++k) {
   const u32 o= key[so[k]];
   chlab[k]= o == key[kadj[sj[k]]] ? korig[NIL - o] : NIL;
  }
 }
 // _tecc2.hpp の gabow_rec と同じ。核の頂点の次数はどれも 2 以上。
 void gabow_rec(u32 nk, const u32* adj, u32* st, u32* bs, const R* rec, FG* fs) {
  u32 top= 0, bt= 0;
  for(u32 r= 0; r < nk; ++r) {
   if(key[r]) continue;
   const R c= rec[r];
   u32 v= r, i= c.s, e= c.e, s0= c.s, n0= c.n0, n1= c.n1, par= NIL;
   st[top++]= v, key[v]= top, bs[bt++]= top;
   u32 pv= top;
   FG* sp= fs;
   for(;;) {
    if(i != e) {
     const u32 w= i == s0 ? n0 : i == s0 + 1 ? n1 : adj[i];
     ++i;
     if(w == par) {
      par= NIL;
      continue;
     }
     const u32 kw= key[w];
     if(!kw) {
      *sp++= {v, i, e, par, pv};
      const R cw= rec[w];
      par= v, v= w, i= s0= cw.s, e= cw.e, n0= cw.n0, n1= cw.n1;
      const u32 m2= -u32(e - i == 2);
      n0^= (n1 ^ par) & m2, e-= m2 & 1, par|= m2;
      st[top++]= v, key[v]= pv= top, bs[bt++]= top;
     } else
      while(kw < bs[bt - 1]) --bt;
    } else {
     if(pv == bs[bt - 1]) {
      --bt;
      while(top >= pv) key[st[--top]]= NIL - v;
     }
     if(sp == fs) break;
     --sp, v= sp->v, i= sp->i, e= sp->e, par= sp->par, pv= sp->pv, s0= NIL - 1;
    }
   }
  }
 }
 int comp(int v) const {
  const u32 w= W[v], t= w >> 29;
  if(t == 5) return int(korig[NIL - key[w & MASK]]);
  if(t == 6) {
   const u32 c= chlab[w & MASK];
   return int(c == NIL ? u32(v) : c);
  }
  return v;
 }
};
}
