#pragma once
// 支配木の核。SEMI-NCA で、作業領域は 2 MB 境界の 1 本の領域で huge page を頼み、配列は u32。s から反復の DFS で届く頂点に行きがけ順の
// 番号を振り、届く頂点どうしの辺だけで逆向きの CSR を番号の上で組み直す (本体のループは番号の順に逆向きの辺の列をなめるだけになり、
// 辺の始点の番号を引き直さずに済む)。逆向きの CSR を組むときは、1 回目の走査で引いた端の番号の組を辺の順に並べて残し、2 回目の
// 走査は番号を引き直さずにそれを読む。
//
// 準支配点 (sdom) は Lengauer–Tarjan と同じく番号の大きい順に、逆向きの辺の始点の eval の最小で求める。森の各点には、森の親と、道の上の
// sdom の最小の値そのもの (頂点でなく値) を 1 つの配列に並べて持ち、eval は根までの道を圧縮しながら最小の値を返す。森の根 (まだ処理して
// いない頂点と s) の sdom は番号そのものなので、根なら番号を返す。直接支配点は、番号の小さい順に、木の親から始めて sdom より大きい
// あいだ直接支配点をたどって決める。
//
// ALG = 3 は、DFS の隣接を順向きの CSR にする。ALG = 4 は、yosupo-scc の gabow_split と同じく、頂点ごとに最初の出辺の行き先と残りの
// 出辺の連結リストの先頭を別々の配列に置き、辺の列を 1 回なめて組む (DFS が下りる 1 歩が読み込み 1 段で済む)。ALG = 7 は ALG = 4 の
// eval を、道を 1 回だけ登る経路半減にする。ALG = 6 は ALG = 7 に、sdom の走査で 8 個先 (番号の 8 小さい頂点) の逆向きの辺の始点の
// 行の先読みを足したもの。
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
template <int ALG> struct Solver {
 struct AL {
  u32 anc, lab;
 };
 int n, s;
 const vector<array<int, 2>>& es;
 Mem mem;
 u32 *num, *vert, *idom;  // num[v] は頂点 v の番号 (届かなければ NIL)、vert[i] は番号 i の頂点、idom[i] は番号 i の直接支配点の番号
 Solver(int n, int s, const vector<array<int, 2>>& es): n(n), s(s), es(es) {}
 void run() {
  const u32 N= n;
  const size_t m= es.size();
  mem.reserve(12 * Mem::words(N + 2) + 2 * Mem::words(2 * m + 2) + 2 * Mem::words(m + 1) + Mem::words(3 * size_t(N)));
  u32 *ro= mem.take(N + 2), *ra= mem.take(m + 1);
  num= mem.take(N + 1), vert= mem.take(N + 1), idom= mem.take(N + 1);
  u32 *par= mem.take(N + 1), *fs= mem.take(3 * size_t(N));
  memset(num, 0xff, N * sizeof(u32));
  u32 R= 0;
  if constexpr(ALG == 3) {
   // 順向きの CSR。fo[v] は v までの出次数の累積で、辺を置くたびに減らして始まりにする。フレームは {頂点, 次に見る辺の位置}。
   u32 *fo= mem.take(N + 2), *fa= mem.take(m + 1);
   memset(fo, 0, (N + 1) * sizeof(u32));
   for(auto& e: es) ++fo[e[0]];
   for(u32 v= 1; v <= N; ++v) fo[v]+= fo[v - 1];
   for(size_t j= m; j--;) fa[--fo[es[j][0]]]= es[j][1];
   u32* sp= fs;
   num[s]= R, par[R]= 0, vert[R++]= s;
   u32 v= s, i= fo[s];
   for(;;) {
    if(i != fo[v + 1]) {
     const u32 w= fa[i++];
     if(num[w] != NIL) continue;
     sp[0]= v, sp[1]= i, sp+= 2;
     num[w]= R, par[R]= num[v], vert[R++]= w;
     v= w, i= fo[w];
    } else {
     if(sp == fs) break;
     sp-= 2, v= sp[0], i= sp[1];
    }
   }
  } else {
   // 最初の行き先 fst と残りの連結リストの先頭 hd。その頂点の最初の辺かどうかはマスクで選ぶ。フレームは {頂点, 残りの連結リストの
   // うちまだ見ていない先頭, 頂点の番号}。nf はいまの頂点の最初の行き先のうちまだ見ていないもの。
   u32 *fst= mem.take(N + 1), *hd= mem.take(N + 1), *nx= mem.take(2 * m + 2);
   memset(fst, 0xff, N * sizeof(u32)), memset(hd, 0xff, N * sizeof(u32));
   for(size_t j= 0; j < m; ++j) {
    const u32 a= es[j][0], b= es[j][1], f= fst[a], h= hd[a], k= -u32(f == NIL);
    nx[2 * j]= b, nx[2 * j + 1]= h;
    fst[a]= (b & k) | (f & ~k), hd[a]= (h & k) | (u32(j) & ~k);
   }
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
  // 逆向きの CSR を番号の上で組む。届かない頂点が端の辺は番兵 R に数えて末尾に捨てる。
  memset(ro, 0, (R + 1) * sizeof(u32));
  u32* tp= mem.take(2 * m + 2);
  for(size_t j= 0; j < m; ++j) {
   const u32 a= num[es[j][0]], b= num[es[j][1]], ok= (a != NIL) & (b != NIL);
   tp[2 * j]= a, tp[2 * j + 1]= ok ? b : R;
   ++ro[ok ? b : R];
  }
  for(u32 k= 1; k <= R; ++k) ro[k]+= ro[k - 1];
  for(size_t j= m; j--;) ra[--ro[tp[2 * j + 1]]]= tp[2 * j];
  snca(R, ro, ra, par);
 }
 // ALG = 6 の eval。道を 1 回登りながら、各点の親を祖父に付け替え (経路半減)、付け替えた点の値に飛ばした親の値を入れる。各点の値は
 // 「その点から親の手前までの最小」のままなので、登った点の値の最小が、v から根の手前までの最小になる。スタックが要らない。
 static u32 eval_halve(u32 v, AL* al) {
  if(al[v].anc == NIL) return v;
  u32 x= v, res= NIL;
  for(;;) {
   const u32 a= al[x].anc;
   if(al[a].anc == NIL) return std::min(res, al[x].lab);  // x の親が根なら、x の値が根の手前までを表す
   if(al[a].lab < al[x].lab) al[x].lab= al[a].lab;
   al[x].anc= al[a].anc, res= std::min(res, al[x].lab), x= al[x].anc;
   if(al[x].anc == NIL) return res;  // 祖父が根なら、登った点の値で根の手前まで尽くしている
  }
 }
 static u32 eval(u32 v, AL* al, u32* path) {
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
 void snca(u32 R, const u32* ro, const u32* ra, const u32* par) {
  AL* al= reinterpret_cast<AL*>(mem.take(2 * size_t(R) + 2));
  u32 *sdom= mem.take(R + 1), *path= mem.take(R + 1);
  for(u32 k= 0; k < R; ++k) al[k]= {NIL, k};
  for(u32 w= R; --w >= 1;) {
   if constexpr(ALG == 6) {
    const u32 z= w > 8 ? w - 8 : 0;
    for(u32 j= ro[z]; j < ro[z + 1]; ++j) __builtin_prefetch(al + ra[j]);
   }
   u32 sw= w;
   for(u32 j= ro[w]; j < ro[w + 1]; ++j) {
    if constexpr(ALG >= 6) sw= std::min(sw, eval_halve(ra[j], al));
    else sw= std::min(sw, eval(ra[j], al, path));
   }
   sdom[w]= sw, al[w]= {par[w], sw};
  }
  idom[0]= 0;
  for(u32 w= 1; w < R; ++w) {
   u32 d= par[w];
   while(d > sdom[w]) d= idom[d];
   idom[w]= d;
  }
 }
 int parent(int v) const {
  const u32 x= num[v];
  return x == NIL ? -1 : int(vert[idom[x]]);
 }
};
}
