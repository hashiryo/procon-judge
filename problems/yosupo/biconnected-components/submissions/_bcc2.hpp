#pragma once
// 二重連結成分の核の 2 本目。_bcc.hpp から、剥がしと CSR を組むところを分岐なしにし、2-core の頂点を詰め、記録の形の DFS で
// 親への辺を番号の比較で飛ばす。
//
// 剥がしは、次数と残っている隣接の XOR を持つところは _bcc.hpp と同じで、積むかどうかを分岐にしない (毎回 q の末尾に書き、
// 次数がちょうど 1 になったときだけ末尾を進める)。待ち行列から出した頂点の次数が 0 (残りの隣接がもう剥がれた) のときも、
// 成分を置くかどうかと、隣接の次数を減らすかどうかを、次数の値 (0 か 1) を掛けて選ぶ。次数が 0 の頂点の XOR は 0 なので、
// 隣接は頂点 0 になり、0 を足し引きするだけで済む。16 個先の頂点の XOR と次数、8 個先の頂点の隣接の XOR と次数を先読みする。
// AOS なら、次数と XOR を 1 つの配列に並べて置き、1 頂点の読み書きを 1 本のキャッシュラインで済ませる。
//
// 剥がしたあと、2-core の頂点に 0 から番号を振り直し (詰める)、DFS の配列をその頂点の数で持つ。詰めた番号は頂点の番号の順。
// 成分には詰めた番号のまま置き、answer() で元の番号に直す。CSR を組むとき、端が剥がれた辺は捨てる。捨てるかどうかも分岐にせず、
// 捨てる辺は番兵の頂点への辺として隣接の配列の末尾 (2-core の辺の後ろ) に書く。
//
// DFS は頂点ごとの記録 {CSR の始まり, 終わり, 最初の隣接, 2 番目の隣接} で回す。下りた直後は、親への辺を番号の比較で飛ばす
// (親の訪問順を読まずに済み、どちらの隣接が親かの分岐の予測が外れても、読み込みを待たない)。親への辺を飛ばしても
// low[v] >= ord[親] の判定は変わらないので、多重辺もすべて飛ばしてよい。次数 2 の頂点は、親でない方の隣接 n0 ^ n1 ^ 親 だけを
// 持つ頂点として扱う (2 本とも親への辺なら親になり、後ろ向きの辺として扱われるが、判定は変わらない)。閉路の頂点では、DFS が
// 下りる 1 歩が記録と訪問順の読み込み 1 段で済む。子から帰ったあとの隣接は adj から読む (記録の n0、n1 は adj の先頭 2 つの写し)。
// フレームには、頂点、位置、終わり、lowlink、訪問順を持つ。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace bcc2 {
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
template <bool AOS= false> struct Solver {
 struct R {
  u32 s, e, n0, n1;
 };
 struct FR {
  u32 v, i, e, lv, ov;
 };
 int n;
 const vector<array<int, 2>>& es;
 Mem mem;
 u32 *bv, *bo, nb= 0;  // 成分 k の頂点は bv[bo[k]] から bv[bo[k + 1] - 1]
 u32 nv0= 0, *orig= nullptr;  // bv の nv0 から後ろは詰めた番号で、元の番号は orig[x]
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(8 * Mem::words(N + 2) + Mem::words(2 * m + 2) + 2 * Mem::words(N + m + 2) + Mem::words(5 * size_t(N)) + Mem::words(4 * size_t(N)));
  // AOS なら dg[2 v] が次数、dg[2 v + 1] が XOR。そうでなければ dg が次数で xr が XOR。
  u32 *dg, *xr, *id;
  if constexpr(AOS) dg= mem.take(2 * (N + 1)), xr= nullptr, id= mem.take(N + 1);
  else dg= mem.take(N + 1), xr= mem.take(N + 1), id= xr;
  auto D= [&](u32 v) -> u32& {
   if constexpr(AOS) return dg[2 * v];
   else return dg[v];
  };
  auto X= [&](u32 v) -> u32& {
   if constexpr(AOS) return dg[2 * v + 1];
   else return xr[v];
  };
  u32 *q= mem.take(N + 1), *off= mem.take(N + 2), *adj= mem.take(2 * m + 2), *ord= mem.take(N + 1), *st= mem.take(N + 1);
  bv= mem.take(N + m + 2), bo= mem.take(N + m + 2);
  u32* fs= mem.take(5 * size_t(N));
  if constexpr(AOS) memset(dg, 0, 2 * size_t(N) * sizeof(u32));
  else memset(dg, 0, N * sizeof(u32)), memset(xr, 0, N * sizeof(u32));
  for(auto& e: es) ++D(e[0]), ++D(e[1]), X(e[0])^= e[1], X(e[1])^= e[0];
  // 辺を持たない頂点は 1 頂点の成分。成分を置くかどうかも分岐にせず、毎回書いて、置くときだけ末尾を進める。
  u32 nv= 0, qt= 0;
  bo[0]= 0;
  for(u32 v= 0; v < N; ++v) {
   const u32 d= D(v);
   bv[nv]= v, nv+= d == 0, bo[nb + 1]= nv, nb+= d == 0, q[qt]= v, qt+= d == 1;
  }
  for(u32 qh= 0; qh < qt; ++qh) {
   const u32 a= q[std::min(qh + 16, qt - 1)];
   __builtin_prefetch(&X(a)), __builtin_prefetch(&D(a));
   const u32 ub= X(q[std::min(qh + 8, qt - 1)]);
   __builtin_prefetch(&X(ub)), __builtin_prefetch(&D(ub));
   const u32 v= q[qh], live= D(v), u= X(v);
   bv[nv]= v, bv[nv + 1]= u, nv+= 2 * live, bo[nb + 1]= nv, nb+= live;
   D(v)= 0, X(u)^= v & -live;
   const u32 du= D(u) - live;
   D(u)= du, q[qt]= u, qt+= live & (du == 1);
  }
  // CSR。off[x] は x までの次数の累積で、辺を置くたびに減らして off[x] を x の始まりにする。番兵の頂点 n2 の off は 2 m から
  // 下がって、2-core の辺の終わりで止まる。剥がした頂点の次数は 0。
  orig= q;  // q は剥がしで使い終わった
  u32 s= 0, n2= 0;
  for(u32 v= 0; v < N; ++v) {
   const u32 d= D(v), c= -u32(d >= 2);
   id[v]= n2 | ~c, orig[n2]= v, s+= d, off[n2]= s, n2-= c;
  }
  off[n2]= 2 * m;
  for(auto& e: es) {
   const u32 ia= id[e[0]], ib= id[e[1]], sk= -u32((ia == NIL) | (ib == NIL)), a= (ia & ~sk) | (n2 & sk), b= (ib & ~sk) | (n2 & sk);
   adj[--off[a]]= b, adj[--off[b]]= a;
  }
  nv0= nv;
  memset(ord, 0, n2 * sizeof(u32));
  R* rec= reinterpret_cast<R*>(mem.take(4 * size_t(N)));
  for(u32 x= 0; x < n2; ++x) {
   const u32 s= off[x], e= off[x + 1];
   rec[x]= {s, e, s < e ? adj[s] : NIL, s + 1 < e ? adj[s + 1] : NIL};
  }
  dfs_rec(n2, adj, ord, st, rec, reinterpret_cast<FR*>(fs), nv);
 }
 // いまの頂点の隣接の位置 i が s0 なら n0、s0 + 1 なら n1、それ以外は adj[i]。子から帰ったら s0 を NIL - 1 にして、adj から読む。
 // 親への辺は下りた直後だけ飛ばし、子から帰ったあとは後ろ向きの辺として扱う (判定は変わらない)。
 void dfs_rec(u32 n2, const u32* adj, u32* ord, u32* st, const R* rec, FR* fs, u32 nv) {
  auto close= [&] { bo[++nb]= nv; };
  u32 idx= 0, top= 0;
  for(u32 r= 0; r < n2; ++r) {
   if(ord[r]) continue;
   const R c= rec[r];
   if(c.s == c.e) continue;
   u32 v= r, i= c.s, e= c.e, s0= c.s, n0= c.n0, n1= c.n1, par= NIL, lv= ++idx, ov= lv;
   FR* sp= fs;
   ord[v]= ov, st[top++]= v;
   for(;;) {
    if(i != e) {
     const u32 w= i == s0 ? n0 : i == s0 + 1 ? n1 : adj[i];
     ++i;
     if(w == par) continue;
     const u32 ow= ord[w];
     if(!ow) {
      *sp++= {v, i, e, lv, ov};
      const R cw= rec[w];
      par= v, v= w, i= s0= cw.s, e= cw.e, n0= cw.n0, n1= cw.n1;
      const u32 m2= -u32(e - i == 2);
      n0^= (n1 ^ par) & m2, e-= m2 & 1, par|= m2;
      lv= ov= ord[v]= ++idx, st[top++]= v;
     } else if(ow < lv) lv= ow;
    } else {
     if(sp == fs) {
      --top;
      break;
     }
     const u32 lc= lv, ch= v;
     --sp, v= sp->v, i= sp->i, e= sp->e, lv= sp->lv, ov= sp->ov, s0= NIL - 1, par= NIL;
     if(lc >= ov) {
      u32 x;
      do x= st[--top], bv[nv++]= x;
      while(x != ch);
      bv[nv++]= v, close();
     } else if(lc < lv) lv= lc;
    }
   }
  }
 }
 vector<vector<int>> answer() const {
  vector<vector<int>> ret(nb);
  for(u32 k= 0; k < nb; ++k) {
   auto& b= ret[k];
   b.resize(bo[k + 1] - bo[k]);
   for(u32 j= bo[k]; j < bo[k + 1]; ++j) b[j - bo[k]]= int(j >= nv0 ? orig[bv[j]] : bv[j]);
  }
  return ret;
 }
};
}
