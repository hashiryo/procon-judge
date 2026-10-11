#pragma once
// 支配木の核。作業領域は 2 MB 境界の 1 本の領域で huge page を頼み、配列は u32。順向きの辺の CSR を組んで s から反復の DFS で
// 届く頂点に番号を振り、届く頂点どうしの辺だけで逆向きの CSR を番号の上で組み直す (本体のループは番号の順に逆向きの辺の列を
// なめるだけになり、辺の始点の番号を引き直さずに済む)。
//
// ALG = 0 は経路圧縮だけの Lengauer–Tarjan (lt.hpp と同じ手順)。番号は行きがけ順。
// ALG = 1 は SEMI-NCA。準支配点 (sdom) は Lengauer–Tarjan と同じく番号の大きい順に eval で求めるが、森の各点には道の上の sdom の
// 最小の値そのものを持つ (頂点でなく値)。直接支配点は、番号の小さい順に、木の親から始めて sdom より大きいあいだ直接支配点を
// たどって決める。バケットが要らない。
// ALG = 2 は Cooper–Harvey–Kennedy の繰り返しの形。番号は帰りがけ順の逆 (s が 0)。番号の小さい順に、直接支配点の決まった
// 逆向きの辺の始点どうしの共通の祖先 (番号の大きい方を直接支配点で登る) を取り、変わらなくなるまで繰り返す。最悪は 2 乗だが、
// 繰り返しの回数が少なければ速い。
// STOP が 0 でなければ診断用で、run() を STOP の段 (1: DFS まで、2: 逆向きの CSR まで、3: ALG = 3 の sdom の走査まで) で打ち切り、
// 最初の parent() で同じ作業領域を使って全部を回し直す。
// ALG = 3 は SEMI-NCA で、eval の森の親と最小の値を 1 つの配列に並べたもの。ALG = 4 はそれに、DFS の隣接を最初の行き先と残りの
// 連結リストに分けて置く形を足したもの。TMP なら、逆向きの CSR を組むときに辺の端の番号を
// 1 回だけ引く。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <cstring>
#include "pj.hpp"
namespace dom {
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
template <int ALG, bool TMP= false, int STOP= 0> struct Solver {
 int n, s;
 const vector<array<int, 2>>& es;
 Mem mem;
 bool partial= false, cut= STOP != 0;  // 診断用: cut なら STOP の段で run() を打ち切り、partial を立てる
 u32 *num, *vert, *idom;  // num[v] は頂点 v の番号 (届かなければ NIL)、vert[i] は番号 i の頂点、idom[i] は番号 i の直接支配点の番号
 Solver(int n, int s, const vector<array<int, 2>>& es): n(n), s(s), es(es) {}
 void run() {
  if constexpr(ALG == 4) {
   run_split();
   return;
  }
  const u32 N= n;
  const size_t m= es.size();
  if(!mem.base) mem.reserve(14 * Mem::words(N + 2) + 2 * Mem::words(m + 1) + Mem::words(2 * size_t(N)) + (TMP ? Mem::words(2 * m + 2) : 0));
  mem.used= 0;
  u32 *fo= mem.take(N + 2), *fa= mem.take(m + 1), *ro= mem.take(N + 2), *ra= mem.take(m + 1);
  num= mem.take(N + 1), vert= mem.take(N + 1), idom= mem.take(N + 1);
  u32 *par= mem.take(N + 1), *fs= mem.take(2 * size_t(N));
  // 順向きの CSR。fo[v] は v までの出次数の累積で、辺を置くたびに減らして始まりにする。
  memset(fo, 0, (N + 1) * sizeof(u32));
  for(auto& e: es) ++fo[e[0]];
  for(u32 v= 1; v <= N; ++v) fo[v]+= fo[v - 1];
  for(size_t j= m; j--;) fa[--fo[es[j][0]]]= es[j][1];
  // 反復の DFS。fs はフレーム {頂点, 次に見る辺の位置}。ALG = 2 では帰りがけ順を vert に積み、あとで逆にする。
  memset(num, 0xff, N * sizeof(u32));
  u32 R= 0, po= 0;
  {
   u32* sp= fs;
   num[s]= R, par[R]= 0, vert[R++]= s;
   u32 v= s, i= fo[s];
   for(;;) {
    if(i != fo[v + 1]) {
     const u32 w= fa[i++];
     if(num[w] != NIL) continue;
     sp[0]= v, sp[1]= i, sp+= 2;
     if constexpr(ALG == 2) num[w]= R++;
     else num[w]= R, par[R]= num[v], vert[R++]= w;
     v= w, i= fo[w];
    } else {
     if constexpr(ALG == 2) vert[po++]= v;
     if(sp == fs) break;
     sp-= 2, v= sp[0], i= sp[1];
    }
   }
  }
  if(STOP == 1 && cut) {
   partial= true;
   return;
  }
  if constexpr(ALG == 2) {
   // 帰りがけ順の逆に番号を振り直す (s が 0)。
   for(u32 k= 0; k < R / 2; ++k) std::swap(vert[k], vert[R - 1 - k]);
   for(u32 k= 0; k < R; ++k) num[vert[k]]= k;
  }
  // 逆向きの CSR を番号の上で組む。届かない頂点が端の辺は捨てる。TMP なら、1 回目の走査で引いた端の番号の組を辺の順に
  // 並べて残し、2 回目の走査は番号を引き直さずにそれを読む。
  memset(ro, 0, (R + 1) * sizeof(u32));
  if constexpr(TMP) {
   u32* tp= mem.take(2 * m + 2);
   for(size_t j= 0; j < m; ++j) {
    const u32 a= num[es[j][0]], b= num[es[j][1]], ok= (a != NIL) & (b != NIL);
    tp[2 * j]= a, tp[2 * j + 1]= ok ? b : R;  // 捨てる辺は番兵 R に数える
    ++ro[ok ? b : R];
   }
   for(u32 k= 1; k <= R; ++k) ro[k]+= ro[k - 1];
   for(size_t j= m; j--;) ra[--ro[tp[2 * j + 1]]]= tp[2 * j];
  } else {
   for(auto& e: es) {
    const u32 a= num[e[0]], b= num[e[1]];
    if(a != NIL && b != NIL) ++ro[b];
   }
   for(u32 k= 1; k <= R; ++k) ro[k]+= ro[k - 1];
   for(auto& e: es) {
    const u32 a= num[e[0]], b= num[e[1]];
    if(a != NIL && b != NIL) ra[--ro[b]]= a;
   }
  }
  if(STOP == 2 && cut) {
   partial= true;
   return;
  }
  if constexpr(ALG == 0) lt(R, ro, ra, par);
  else if constexpr(ALG == 1) snca(R, ro, ra, par);
  else if constexpr(ALG == 2) chk(R, ro, ra);
  else snca_pk(R, ro, ra, par);
 }
 // 経路圧縮。x から森の根の子まで登り、上から順に lab と anc を直す。LAB が真なら lab は sdom の最小の頂点の番号、偽なら値。
 template <bool VERT> static u32 eval(u32 v, u32* anc, u32* lab, const u32* sdom, u32* path) {
  if(anc[v] == NIL) return VERT ? v : sdom[v];
  u32 k= 0;
  for(u32 x= v; anc[anc[x]] != NIL; x= anc[x]) path[k++]= x;
  while(k--) {
   const u32 x= path[k], a= anc[x];
   if constexpr(VERT) {
    if(sdom[lab[a]] < sdom[lab[x]]) lab[x]= lab[a];
   } else {
    if(lab[a] < lab[x]) lab[x]= lab[a];
   }
   anc[x]= anc[a];
  }
  return lab[v];
 }
 void lt(u32 R, const u32* ro, const u32* ra, const u32* par) {
  u32 *sdom= mem.take(R + 1), *anc= mem.take(R + 1), *lab= mem.take(R + 1), *bh= mem.take(R + 1), *bn= mem.take(R + 1), *path= mem.take(R + 1);
  for(u32 k= 0; k < R; ++k) sdom[k]= lab[k]= k, anc[k]= bh[k]= NIL;
  for(u32 w= R; --w >= 1;) {
   u32 sw= sdom[w];
   for(u32 j= ro[w]; j < ro[w + 1]; ++j) sw= std::min(sw, sdom[eval<true>(ra[j], anc, lab, sdom, path)]);
   sdom[w]= sw, bn[w]= bh[sw], bh[sw]= w;
   const u32 p= par[w];
   anc[w]= p;
   for(u32 v= bh[p]; v != NIL; v= bn[v]) {
    const u32 u= eval<true>(v, anc, lab, sdom, path);
    idom[v]= sdom[u] < sdom[v] ? u : p;
   }
   bh[p]= NIL;
  }
  idom[0]= 0;
  for(u32 w= 1; w < R; ++w)
   if(idom[w] != sdom[w]) idom[w]= idom[idom[w]];
 }
 void snca(u32 R, const u32* ro, const u32* ra, const u32* par) {
  u32 *sdom= mem.take(R + 1), *anc= mem.take(R + 1), *lab= mem.take(R + 1), *path= mem.take(R + 1);
  for(u32 k= 0; k < R; ++k) sdom[k]= lab[k]= k, anc[k]= NIL;
  for(u32 w= R; --w >= 1;) {
   u32 sw= sdom[w];
   for(u32 j= ro[w]; j < ro[w + 1]; ++j) sw= std::min(sw, eval<false>(ra[j], anc, lab, sdom, path));
   sdom[w]= lab[w]= sw, anc[w]= par[w];
  }
  idom[0]= 0;
  for(u32 w= 1; w < R; ++w) {
   u32 d= par[w];
   while(d > sdom[w]) d= idom[d];
   idom[w]= d;
  }
 }
 // ALG = 3 は SEMI-NCA で、森の親と道の上の sdom の最小の値を 1 つの配列に並べる (経路圧縮で 1 頂点を 1 本のキャッシュラインで
 // 読み書きする)。森の根 (まだ処理していない頂点と s) の sdom は番号そのものなので、eval は根なら番号を返す。
 struct AL {
  u32 anc, lab;
 };
 static u32 eval_pk(u32 v, AL* al, u32* path) {
  if(al[v].anc == NIL) return v;
  u32 k= 0;
  for(u32 x= v; al[al[x].anc].anc != NIL; x= al[x].anc) path[k++]= x;
  while(k--) {
   const u32 x= path[k], a= al[x].anc;
   if(al[a].lab < al[x].lab) al[x].lab= al[a].lab;
   al[x].anc= al[a].anc;
  }
  return al[v].lab;
 }
 void snca_pk(u32 R, const u32* ro, const u32* ra, const u32* par) {
  AL* al= reinterpret_cast<AL*>(mem.take(2 * size_t(R) + 2));
  u32 *sdom= mem.take(R + 1), *path= mem.take(R + 1);
  for(u32 k= 0; k < R; ++k) al[k]= {NIL, k};
  for(u32 w= R; --w >= 1;) {
   u32 sw= w;
   for(u32 j= ro[w]; j < ro[w + 1]; ++j) sw= std::min(sw, eval_pk(ra[j], al, path));
   sdom[w]= sw, al[w]= {par[w], sw};
  }
  if(STOP == 3 && cut) {
   partial= true;
   return;
  }
  idom[0]= 0;
  for(u32 w= 1; w < R; ++w) {
   u32 d= par[w];
   while(d > sdom[w]) d= idom[d];
   idom[w]= d;
  }
 }
 void chk(u32 R, const u32* ro, const u32* ra) {
  for(u32 k= 0; k < R; ++k) idom[k]= NIL;
  idom[0]= 0;
  for(bool changed= true; changed;) {
   changed= false;
   for(u32 w= 1; w < R; ++w) {
    u32 d= NIL;
    for(u32 j= ro[w]; j < ro[w + 1]; ++j) {
     u32 a= ra[j];
     if(a == w || idom[a] == NIL) continue;  // 自己ループは支配に関わらない
     if(d == NIL) {
      d= a;
      continue;
     }
     while(a != d) {
      while(a > d) a= idom[a];
      while(d > a) d= idom[d];
     }
    }
    if(idom[w] != d) idom[w]= d, changed= true;
   }
  }
 }
 // ALG = 4 は ALG = 3 の SEMI-NCA で、DFS の隣接を yosupo-scc の gabow_split と同じ形にする。頂点ごとに最初の出辺の行き先 fst と、
 // 残りの出辺の連結リストの先頭 hd を別々の 4 byte の配列に置き、辺の列を 1 回なめて組む (その頂点の最初の辺かどうかはマスクで
 // 選ぶ)。DFS が下りる 1 歩は、行き先の fst と hd と訪問の印の読み込み 1 段で済む。逆向きの CSR は TMP と同じく番号の組を残して
 // 組む。
 void run_split() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(12 * Mem::words(N + 2) + Mem::words(2 * m + 2) + 2 * Mem::words(m + 1) + Mem::words(3 * size_t(N)) + Mem::words(2 * m + 2));
  u32 *fst= mem.take(N + 1), *hd= mem.take(N + 1), *nx= mem.take(2 * m + 2), *ro= mem.take(N + 2), *ra= mem.take(m + 1);
  num= mem.take(N + 1), vert= mem.take(N + 1), idom= mem.take(N + 1);
  u32 *par= mem.take(N + 1), *fs= mem.take(3 * size_t(N));
  memset(fst, 0xff, N * sizeof(u32)), memset(hd, 0xff, N * sizeof(u32));
  for(size_t j= 0; j < m; ++j) {
   const u32 a= es[j][0], b= es[j][1], f= fst[a], h= hd[a], k= -u32(f == NIL);
   nx[2 * j]= b, nx[2 * j + 1]= h;
   fst[a]= (b & k) | (f & ~k), hd[a]= (h & k) | (u32(j) & ~k);
  }
  // 反復の DFS。フレームは {頂点, 残りの連結リストのうちまだ見ていない先頭, 頂点の番号}。nf はいまの頂点の最初の行き先のうち
  // まだ見ていないもの。
  memset(num, 0xff, N * sizeof(u32));
  u32 R= 0;
  {
   u32* sp= fs;
   num[s]= R, par[R]= 0, vert[R++]= s;
   u32 v= s, iv= 0, nf= fst[s], ce= hd[s];
   for(;;) {
    u32 w;
    if(nf != NIL) w= nf, nf= NIL;
    else if(ce != NIL) w= nx[2 * ce], ce= nx[2 * ce + 1];
    else {
     if(sp == fs) break;
     sp-= 3, v= sp[0], ce= sp[1], iv= sp[2];
     continue;
    }
    const u32 nw= num[w], fw= fst[w], hw= hd[w];
    if(nw != NIL) continue;
    sp[0]= v, sp[1]= ce, sp[2]= iv, sp+= 3;
    num[w]= R, par[R]= iv, vert[R]= w, iv= R++, v= w, nf= fw, ce= hw;
   }
  }
  memset(ro, 0, (R + 1) * sizeof(u32));
  u32* tp= mem.take(2 * m + 2);
  for(size_t j= 0; j < m; ++j) {
   const u32 a= num[es[j][0]], b= num[es[j][1]], ok= (a != NIL) & (b != NIL);
   tp[2 * j]= a, tp[2 * j + 1]= ok ? b : R;
   ++ro[ok ? b : R];
  }
  for(u32 k= 1; k <= R; ++k) ro[k]+= ro[k - 1];
  for(size_t j= m; j--;) ra[--ro[tp[2 * j + 1]]]= tp[2 * j];
  snca_pk(R, ro, ra, par);
 }
 int parent(int v) const {
  if(partial) {
   // 診断用。打ち切った run() を、同じ作業領域で最後まで回し直す (計測区間の外)。
   auto* self= const_cast<Solver*>(this);
   self->cut= false, self->partial= false, self->run();
  }
  const u32 x= num[v];
  return x == NIL ? -1 : int(vert[idom[x]]);
 }
};
}
