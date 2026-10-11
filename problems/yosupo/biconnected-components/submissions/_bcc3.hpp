#pragma once
// 二重連結成分の核の 3 本目。two-edge-connected-components の _tecc3.hpp と同じく、2-core の次数 2 の頂点が続く鎖を、DFS の前に
// 何本も同時に歩いて 1 本の辺に縮め、残った頂点 (核) だけを DFS で分ける。
//
// 剥がしまでは _bcc2.hpp と同じ。剥がしたあと、次数 3 以上の頂点と、次数 2 の頂点のうち番号のハッシュで選んだ 2^-LOG_S ほどの
// 頂点を核にし、核の頂点の CSR を組む。鎖の歩き方と行き先の決め方は _tecc3.hpp と同じで、歩きの番号は鎖の欄の通し番号 k
// (欄の位置は sj[k]、持ち主は so[k]、最初の鎖の頂点は sf[k]、止まった頂点の語は sr[k])。1 本の鎖を代表する歩きは、核に着いた歩きか、
// 出会った 2 本のうち番号の小さい方 (1 歩目でもう訪れた頂点に当たった歩きは、当たった相手が代表)。
//
// 縮めた多重グラフを、_bcc2.hpp の記録の形の Hopcroft–Tarjan で分ける。ただし、橋を見分けるために、親への辺は 1 本だけ飛ばす
// (二辺連結成分と同じ。2 本目からの平行な辺は後ろ向きの辺として low に入る)。頂点ごとに、訪問順 ord、帰るときの low (lowfin)、
// スタックから外されたときの成分 blk を残す。無向グラフの DFS の辺はどれも先祖と子孫を結ぶので、鎖 (o, t) の深い側の端を d とすると、
//   (a) o = t なら、鎖は輪で、鎖の頂点と o だけで 1 つの成分になる。
//   (b) lowfin[d] = ord[d] なら、鎖は縮めた多重グラフの橋で、鎖の辺はどれも橋。鎖の辺ごとに 2 頂点の成分になる。DFS は鎖を 1 本の辺と
//       みて 2 頂点の成分 blk[d] を作っているので、それを鎖の最初の辺に書き換え、残りの辺は剥がしの XOR で鎖を歩き直して置く。
//   (c) そうでなければ、鎖の頂点は成分 blk[d] に入る。
// 鎖の頂点は、歩いたときに残した記録を成分ごとに数えてから並べ (計数の整列)、成分の頂点の列は bv の核の分と cv の鎖の分に分けて
// 持つ。answer() でつなぐ。
// 核を含まない閉路は、_tecc3.hpp と同じく辺の列から隣接を拾って 1 本ずつ歩き、閉路ごとに 1 つの成分にする。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace bcc3 {
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
template <int LOG_S= 5, int LANES= 16> struct Solver {
 struct R {
  u32 s, e, n0, n1;
 };
 struct FR {
  u32 v, i, e, lv, ov, par;
 };
 struct Lane {
  u32 k, prev, cur;
 };
 int n;
 const vector<array<int, 2>>& es;
 Mem mem;
 u32 *bv, *bo, *cv, *co, nb= 0, nb0= 0;  // 成分 b の頂点は bv[bo[b]] から bv[bo[b + 1] - 1] と、b >= nb0 なら cv[co[b - nb0]] から
                                         // cv[co[b - nb0 + 1] - 1]
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(15 * Mem::words(N + 2) + 6 * Mem::words(2 * m + N + 2) + 3 * Mem::words(2 * (N + m) + 4) + Mem::words(10 * size_t(N)));
  u32 *deg= mem.take(N + 1), *xr= mem.take(N + 1), *q= mem.take(N + 1), *W= mem.take(N + 1), *korig= mem.take(N + 1), *koff= mem.take(N + 2);
  u32 *kadj= mem.take(2 * m + 2), *sj= mem.take(2 * m + 2), *so= mem.take(2 * m + 2), *sf= mem.take(2 * m + 2), *sr= mem.take(2 * m + 2), *cblk= mem.take(2 * m + N + 2);
  bv= mem.take(2 * (N + m) + 4), bo= mem.take(2 * (N + m) + 4), co= mem.take(2 * (N + m) + 4), cv= mem.take(N + 1);
  u32* lg= mem.take(N + 1);  // 訪れた鎖の頂点の記録 (訪れた順)
  memset(deg, 0, N * sizeof(u32)), memset(xr, 0, N * sizeof(u32));
  for(auto& e: es) ++deg[e[0]], ++deg[e[1]], xr[e[0]]^= e[1], xr[e[1]]^= e[0];
  // 辺を持たない頂点は 1 頂点の成分。剥がしは _bcc2.hpp と同じ。
  u32 nv= 0, qt= 0;
  auto close= [&] { bo[++nb]= nv; };
  bo[0]= 0;
  for(u32 v= 0; v < N; ++v) {
   const u32 d= deg[v];
   bv[nv]= v, nv+= d == 0, bo[nb + 1]= nv, nb+= d == 0, q[qt]= v, qt+= d == 1;
  }
  for(u32 qh= 0; qh < qt; ++qh) {
   const u32 a= q[std::min(qh + 16, qt - 1)];
   __builtin_prefetch(xr + a), __builtin_prefetch(deg + a);
   const u32 ub= xr[q[std::min(qh + 8, qt - 1)]];
   __builtin_prefetch(xr + ub), __builtin_prefetch(deg + ub);
   const u32 v= q[qh], live= deg[v], u= xr[v];
   bv[nv]= v, bv[nv + 1]= u, nv+= 2 * live, bo[nb + 1]= nv, nb+= live;
   deg[v]= 0, xr[u]^= v & -live;
   const u32 du= deg[u] - live;
   deg[u]= du, q[qt]= u, qt+= live & (du == 1);
  }
  nb0= nb;  // ここより前の成分 (剥がしの成分) には鎖の頂点が入らない
  // 核と鎖の頂点に分け、核の CSR を組む (_tecc3.hpp と同じ)。
  u32 nk= 0, tot= 0, nc= 0;
  for(u32 v= 0; v < N; ++v) {
   const u32 d= deg[v], r= ((v * 0x9E3779B1u) >> (32 - LOG_S)) == 0, mk= -u32((d >= 3) | ((d == 2) & r)), mc= -u32((d == 2) & !r);
   W[v]= ((KER | nk) & mk) | (xr[v] & mc) | (PEELED & ~(mk | mc));
   korig[nk]= v, tot+= d & mk, koff[nk]= tot, nk-= mk, nc-= mc;
  }
  koff[nk]= tot;
  for(auto& e: es) {
   const u32 a= e[0], b= e[1], wa= W[a], wb= W[b];
   const u32 live= u32((wa >> 29) != 7) & u32((wb >> 29) != 7), ka= u32((wa >> 29) == 5), kb= u32((wb >> 29) == 5);
   const u32 da= ka & live, db= kb & live, mka= -ka, mkb= -kb, mda= -da, mdb= -db;
   const u32 va= ((wb & MASK) & mkb) | ((CHAIN | b) & ~mkb), vb= ((wa & MASK) & mka) | ((CHAIN | a) & ~mka);
   const u32 ta= ((wa & MASK) & mda) | (nk & ~mda), tb= ((wb & MASK) & mdb) | (nk & ~mdb);
   const u32 pa= koff[ta] - da;
   kadj[pa]= va, koff[ta]= pa;
   const u32 pb= koff[tb] - db;
   kadj[pb]= vb, koff[tb]= pb;
  }
  u32 ns= 0;
  for(u32 k= 0; k < nk; ++k)
   for(u32 j= koff[k], e= koff[k + 1]; j < e; ++j) sj[ns]= j, so[ns]= k, sf[ns]= kadj[j] & MASK, ns+= kadj[j] >> 31;
  // 鎖を LANES 本ずつ交互に歩く。
  Lane L[LANES];
  u32 nl= 0, si= 0, nvis= 0;
  while(nl < LANES && si < ns) L[nl]= {si, korig[so[si]], sf[si]}, ++nl, ++si;
  while(nl) {
   for(u32 l= 0; l < nl;) {
    Lane& x= L[l];
    const u32 w= W[x.cur];
    if(w >> 31) {
     sr[x.k]= w;
     if(si < ns) x= {si, korig[so[si]], sf[si]}, ++si, ++l;
     else x= L[--nl];
    } else {
     W[x.cur]= VIS | x.k, lg[nvis++]= x.cur;
     const u32 nx= w ^ x.prev;
     x.prev= x.cur, x.cur= nx, ++l;
    }
   }
  }
  // 核を含まない閉路。剥がしていない隣接を 1 つずつ拾い (q を使い回す)、閉路ごとに歩いて、鎖の分だけの成分にする。
  if(nvis != nc) {
   for(auto& e: es) {
    const u32 a= e[0], b= e[1];
    if((W[a] >> 29) == 7 || (W[b] >> 29) == 7) continue;
    if(!(W[a] >> 31)) q[a]= b;
    if(!(W[b] >> 31)) q[b]= a;
   }
   u32 J= ns;
   for(u32 v= 0; v < N; ++v) {
    if(W[v] >> 31) continue;
    cblk[J]= nb, close(), W[v]= VIS | J, lg[nvis++]= v;
    for(u32 prev= v, cur= q[v]; !(W[cur] >> 31);) {
     const u32 w= W[cur];
     W[cur]= VIS | J, lg[nvis++]= cur;
     const u32 nx= w ^ prev;
     prev= cur, cur= nx;
    }
    ++J;
   }
  }
  // 鎖の欄の行き先を核の番号にする。
  for(u32 k= 0; k < ns; ++k) {
   const u32 w= sr[k];
   kadj[sj[k]]= (w >> 29) == 5 ? w & MASK : so[w & MASK];
  }
  R* rec= reinterpret_cast<R*>(mem.take(4 * size_t(N)));
  FR* fs= reinterpret_cast<FR*>(mem.take(6 * size_t(N)));
  u32 *ord= mem.take(N + 1), *lowfin= mem.take(N + 1), *blk= mem.take(N + 1), *st= mem.take(N + 1);
  for(u32 k= 0; k < nk; ++k) {
   const u32 s= koff[k], e= koff[k + 1];
   rec[k]= {s, e, kadj[s], kadj[s + 1]};
  }
  memset(ord, 0, nk * sizeof(u32));
  // 縮めた多重グラフの Hopcroft–Tarjan。核の頂点の次数はどれも 2 以上。
  u32 idx= 0, top= 0;
  for(u32 r= 0; r < nk; ++r) {
   if(ord[r]) continue;
   const R c= rec[r];
   u32 v= r, i= c.s, e= c.e, s0= c.s, n0= c.n0, n1= c.n1, par= NIL, lv= ++idx, ov= lv;
   FR* sp= fs;
   ord[v]= ov, st[top++]= v;
   for(;;) {
    if(i != e) {
     const u32 w= i == s0 ? n0 : i == s0 + 1 ? n1 : kadj[i];
     ++i;
     if(w == par) {
      par= NIL;
      continue;
     }
     const u32 ow= ord[w];
     if(!ow) {
      *sp++= {v, i, e, lv, ov, par};
      const R cw= rec[w];
      par= v, v= w, i= s0= cw.s, e= cw.e, n0= cw.n0, n1= cw.n1;
      const u32 m2= -u32(e - i == 2);
      n0^= (n1 ^ par) & m2, e-= m2 & 1, par|= m2;
      lv= ov= ord[v]= ++idx, st[top++]= v;
     } else if(ow < lv) lv= ow;
    } else {
     lowfin[v]= lv;
     if(sp == fs) {
      --top;
      break;
     }
     const u32 lc= lv, ch= v;
     --sp, v= sp->v, i= sp->i, e= sp->e, lv= sp->lv, ov= sp->ov, par= sp->par, s0= NIL - 1;
     if(lc >= ov) {
      u32 x;
      do x= st[--top], blk[x]= nb, bv[nv++]= korig[x];
      while(x != ch);
      bv[nv++]= korig[v], close();
     } else if(lc < lv) lv= lc;
    }
   }
  }
  // 鎖ごとの成分。cblk[k] は鎖の頂点が入る成分 (橋の鎖なら NIL)。輪の鎖は代表の歩きで成分を作り、代表でない歩きはあとで写す。
  auto canon= [&](u32 k) {
   const u32 w= sr[k];
   if((w >> 29) == 5) return k;
   const u32 p= w & MASK;
   return (sr[p] >> 29) == 5 ? p : std::min(k, p);
  };
  bool loops= false;
  for(u32 k= 0; k < ns; ++k) {
   const u32 o= so[k], t= kadj[sj[k]];
   if(o == t) {
    if(canon(k) == k) bv[nv++]= korig[o], cblk[k]= nb, close();
    else cblk[k]= NIL - 1, loops= true;
    continue;
   }
   const u32 d= ord[o] > ord[t] ? o : t;
   if(lowfin[d] != ord[d]) {
    cblk[k]= blk[d];
    continue;
   }
   cblk[k]= NIL;
   if(canon(k) != k) continue;
   // 橋の鎖。DFS が作った 2 頂点の成分を最初の辺 (o, sf[k]) にし、残りの辺を歩き直して置く。
   const u32 b= blk[d];
   bv[bo[b]]= korig[o], bv[bo[b] + 1]= sf[k];
   for(u32 prev= korig[o], cur= sf[k];;) {
    const u32 nx= xr[cur] ^ prev;
    bv[nv++]= cur, bv[nv++]= nx, close();
    if((W[nx] >> 29) == 5) break;
    prev= cur, cur= nx;
   }
  }
  if(loops)
   for(u32 k= 0; k < ns; ++k)
    if(cblk[k] == NIL - 1) cblk[k]= cblk[canon(k)];
  // 訪れた鎖の頂点を、記録から成分ごとに並べる (成分 nb0 から後ろだけ)。橋の鎖の頂点は、番兵の成分 nb に数えて末尾に捨てる。
  // co[b - nb0] が成分 b の鎖の分の始まり。
  memset(co, 0, (nb - nb0 + 2) * sizeof(u32));
  for(u32 t= 0; t < nvis; ++t) {
   const u32 b= cblk[W[lg[t]] & MASK];
   ++co[b == NIL ? nb - nb0 : b - nb0];
  }
  for(u32 b= 1; b <= nb - nb0; ++b) co[b]+= co[b - 1];
  for(u32 t= nvis; t--;) {
   const u32 v= lg[t], b= cblk[W[v] & MASK];
   cv[--co[b == NIL ? nb - nb0 : b - nb0]]= v;
  }
 }
 vector<vector<int>> answer() const {
  vector<vector<int>> ret(nb);
  for(u32 b= 0; b < nb; ++b) {
   auto& x= ret[b];
   x.assign(bv + bo[b], bv + bo[b + 1]);
   if(b >= nb0) x.insert(x.end(), cv + co[b - nb0], cv + co[b - nb0 + 1]);
  }
  return ret;
 }
};
}
