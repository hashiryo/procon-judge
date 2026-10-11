#pragma once
// 二辺連結成分の核の 2 本目。_tecc.hpp から、剥がしと CSR を組むところを分岐なしにし、2-core の頂点を詰められるようにし、
// 記録の形の DFS で次数 2 の頂点を 1 本の辺として扱う。いつも剥がす。
//
// 剥がしは、積むかどうかを分岐にしない (毎回 q の末尾に書き、次数がちょうど 1 になったときだけ末尾を進める)。待ち行列から出した
// 頂点の次数が 0 のときも、隣接の次数を減らすかどうかを次数の値 (0 か 1) を掛けて選ぶ。次数が 0 の頂点の XOR は 0 なので、隣接は
// 頂点 0 になり、0 を足し引きするだけで済む。16 個先の頂点の XOR と次数、8 個先の頂点の隣接の XOR と次数を先読みする。
// 自己ループは、次数と XOR に足す値を 0 にして分岐なしで捨てる。AOS なら、次数と XOR を 1 つの配列に並べて置き、1 頂点の
// 読み書きを 1 本のキャッシュラインで済ませる。
//
// PEEL = 1 は頂点の番号のままで 2-core の CSR を組む。PEEL = 2 は 2-core の頂点に 0 から番号を振り直し (詰める)、DFS の配列を
// その頂点の数で持つ。端が剥がれた辺と自己ループは、番兵の頂点 D への辺として隣接の配列の末尾に書いて捨てる (分岐なし)。
//
// DFS = 0 は _tecc.hpp の ALG = 0 と同じ CSR の path-based。DFS = 1 は頂点ごとの記録 {CSR の始まり, 終わり, 最初の隣接, 2 番目の
// 隣接} で回し、次数 2 の頂点に下りたら、親でない方の隣接 n0 ^ n1 ^ 親 だけを持つ頂点として扱う (親への木の辺はそこで飛ばした
// ことになる。2 本とも親への辺なら親になり、平行な辺として後ろ向きの辺に扱われる)。子から帰ったあとの隣接は adj から読む。
// フレームには、頂点、位置、終わり、親、スタックでの位置を持つ。
//
// 成分の名前は、PEEL = 1 では DFS で成分を閉じた頂点か剥がした頂点。PEEL = 2 では鍵に詰めた番号を置き、comp() で元の番号に直す。
//
// 段は 4 つ (1: 確保と次数と XOR、2: 剥がし、3: CSR と記録、4: DFS)。STOP が 0 でなければ、run() は STOP の段までで止め、残りを
// 最初の comp() に回す (段ごとの時間を測る診断用。出力は変わらない)。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace tecc2 {
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
template <int PEEL, int DFS, bool AOS= false, int STOP= 0> struct Solver {
 struct F {
  u32 v, i, e, par;
 };
 struct R {
  u32 s, e, n0, n1;
 };
 struct FG {
  u32 v, i, e, par, pv;
 };
 int n;
 const vector<array<int, 2>>& es;
 Mem mem;
 u32 *key, *id= nullptr, *orig= nullptr;  // 鍵は成分に入ったら NIL - (名前)。DFS が触らない頂点の鍵は 0
 u32 *deg, *xr, *q, *off, *adj, *st, *bs, *fs, qt= 0, n2= 0;
 R* rec= nullptr;
 int stage= 0;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 // AOS なら deg[2 v] が次数、deg[2 v + 1] が XOR。
 u32& D(u32 v) {
  if constexpr(AOS) return deg[2 * v];
  else return deg[v];
 }
 u32& X(u32 v) {
  if constexpr(AOS) return deg[2 * v + 1];
  else return xr[v];
 }
 void run() { step(STOP ? STOP : 4); }
 void step(int upto) {
  while(stage < upto) {
   ++stage;
   if(stage == 1) s1();
   else if(stage == 2) s2();
   else if(stage == 3) s3();
   else s4();
  }
 }
 void s1() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(10 * Mem::words(N + 2) + Mem::words(2 * m + 2) + Mem::words(5 * size_t(N)) + (DFS ? Mem::words(4 * size_t(N)) : 0));
  if constexpr(AOS) deg= mem.take(2 * (N + 1)), xr= nullptr;
  else deg= mem.take(N + 1), xr= mem.take(N + 1);
  q= mem.take(N + 1), off= mem.take(N + 2), adj= mem.take(2 * m + 2), st= mem.take(N + 1), bs= mem.take(N + 1);
  key= mem.take(N + 1);
  fs= mem.take(5 * size_t(N));
  if constexpr(AOS) id= mem.take(N + 1), memset(deg, 0, 2 * size_t(N) * sizeof(u32));
  else id= xr, memset(deg, 0, N * sizeof(u32)), memset(xr, 0, N * sizeof(u32));
  for(auto& e: es) {
   const u32 a= e[0], b= e[1], d= a != b;
   D(a)+= d, D(b)+= d, X(a)^= b & -d, X(b)^= a & -d;
  }
  for(u32 v= 0; v < N; ++v) q[qt]= v, qt+= D(v) == 1;
 }
 void s2() {
  for(u32 qh= 0; qh < qt; ++qh) {
   const u32 a= q[std::min(qh + 16, qt - 1)];
   __builtin_prefetch(&X(a)), __builtin_prefetch(&D(a));
   const u32 ub= X(q[std::min(qh + 8, qt - 1)]);
   __builtin_prefetch(&X(ub)), __builtin_prefetch(&D(ub));
   const u32 v= q[qh], live= D(v), u= X(v);
   D(v)= 0, X(u)^= v & -live;
   const u32 du= D(u) - live;
   D(u)= du, q[qt]= u, qt+= live & (du == 1);
  }
 }
 // CSR。off[x] は x までの次数の累積で、辺を置くたびに減らして off[x] を x の始まりにする。D は捨てる辺の行き先の番兵で、
 // off[D] は 2 m から下がって、2-core の辺の終わりで止まる。剥がした頂点の次数は 0。
 void s3() {
  const u32 N= n;
  const size_t m= es.size();
  n2= N;
  if constexpr(PEEL == 2) {
   orig= q;  // q は剥がしで使い終わった
   u32 s= 0, k= 0;
   for(u32 v= 0; v < N; ++v) {
    const u32 d= D(v), c= -u32(d >= 2);
    id[v]= k | ~c, orig[k]= v, s+= d, off[k]= s, k-= c;
   }
   n2= k, off[k]= 2 * m;
   for(auto& e: es) {
    const u32 ia= id[e[0]], ib= id[e[1]], sk= -u32((ia == NIL) | (ib == NIL) | (ia == ib)), a= (ia & ~sk) | (k & sk), b= (ib & ~sk) | (k & sk);
    adj[--off[a]]= b, adj[--off[b]]= a;
   }
  } else {
   u32 s= 0;
   for(u32 v= 0; v < N; ++v) s+= D(v), off[v]= s;
   off[N]= 2 * m;
   for(auto& e: es) {
    const u32 a= e[0], b= e[1], sk= -(u32(D(a) == 0) | u32(D(b) == 0) | u32(a == b)), x= (a & ~sk) | (N & sk), y= (b & ~sk) | (N & sk);
    adj[--off[x]]= y, adj[--off[y]]= x;
   }
  }
  memset(key, 0, n2 * sizeof(u32));
  if constexpr(DFS == 1) {
   rec= reinterpret_cast<R*>(mem.take(4 * size_t(N)));
   for(u32 x= 0; x < n2; ++x) {
    const u32 s= off[x], e= off[x + 1];
    rec[x]= {s, e, s < e ? adj[s] : NIL, s + 1 < e ? adj[s + 1] : NIL};
   }
  }
 }
 void s4() {
  if constexpr(DFS == 0) gabow(reinterpret_cast<F*>(fs));
  else gabow_rec(reinterpret_cast<FG*>(fs));
 }
 // 次数 0 の頂点 (剥がした頂点) は飛ばす。
 void gabow(F* fs) {
  u32 top= 0, bt= 0;
  for(u32 r= 0; r < n2; ++r) {
   if(key[r] || off[r] == off[r + 1]) continue;
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
 // いまの頂点の隣接の位置 i が s0 なら n0、s0 + 1 なら n1、それ以外は adj[i]。子から帰ったら s0 を NIL - 1 にして、adj から読む。
 // pv はいまの頂点のスタックでの位置 (= 鍵)。
 void gabow_rec(FG* fs) {
  u32 top= 0, bt= 0;
  for(u32 r= 0; r < n2; ++r) {
   if(key[r]) continue;
   const R c= rec[r];
   if(c.s == c.e) continue;
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
  const_cast<Solver*>(this)->step(4);
  if constexpr(PEEL == 2) {
   const u32 x= id[v];
   return int(x == NIL ? u32(v) : orig[NIL - key[x]]);
  } else {
   return int(key[v] ? NIL - key[v] : u32(v));
  }
 }
};
}
