#pragma once
#include "_mont.hpp"
// 無向の辺から HLD を作る。作り方を 2 つ書き、テンプレート引数で選ぶ。
//   Build::Xor: 隣接リストを組まず、頂点ごとに次数と隣の番号の xor を持ち、次数 1 の頂点を葉から剥がす。剥がすときの
//               xor が親になる。根は次数に 2 を足して剥がさない。番号の小さい順に見て、剥がした頂点の親の次数が 1 に
//               なったら続けてその親を剥がす。剥がした順は子が親より前に来る。
//   Build::Csr: 隣接の CSR を組み、根から BFS して親を決める。BFS の順を逆にすると子が親より前に来る。
// どちらも、子が親より前に来る順で部分木の大きさと重い子を決め、逆順で重い子を先にした行きがけ順の位置を配る。
// 位置ごとに、鎖の先頭の位置と先頭の親の位置を 8 byte の組 (ch) で持つ。lca の hld_hp_b と同じ持ち方。
// vertex-add-path-sum の _path.hpp の写しで、道を u から v への向きの順に区間へ分ける path_ordered を足した。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
namespace comp_hld {
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
using Buf= std::unique_ptr<int[], FreeDeleter>;
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline Buf alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(int) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return Buf(static_cast<int*>(p));
}
inline u64 chain(int head, int up) { return u64(unsigned(head)) | u64(unsigned(up)) << 32; }
inline int head_of(u64 c) { return int(unsigned(c)); }
inline int up_of(u64 c) { return int(c >> 32); }
enum class Build { Xor, Csr };
struct Tree {
 int n;
 Buf buf;
 int *pos, *szp, *upp;  // 頂点 → 位置、位置 → 部分木の大きさ、位置 → 親の位置 (根は 0)
 u64* ch;              // 位置 → 鎖の組
 template <Build B> static Tree make(int n, const vector<array<int, 2>>& edges, int root= 0) {
  Tree t;
  t.n= n, t.buf= alloc_huge(5 * size_t(n));
  t.ch= (u64*)t.buf.get(), t.pos= (int*)(t.ch + n), t.szp= t.pos + n, t.upp= t.szp + n;  // ch を先頭に置いて 8 byte に揃える
  Buf tmp= alloc_huge(6 * size_t(n) + 2);
  int *ord= tmp.get(), *par= ord + n, *sz= par + n, *hb= sz + n + 1;  // hb は 2n 個
  if constexpr(B == Build::Xor) {
   int* dx= hb;  // dx[2v] = 次数、dx[2v + 1] = 隣の番号の xor
   std::fill(dx, dx + 2 * size_t(n), 0);
   for(const auto& e: edges) {
    const int a= e[0], b= e[1];
    ++dx[2 * a], dx[2 * a + 1]^= b, ++dx[2 * b], dx[2 * b + 1]^= a;
   }
   dx[2 * root]+= 2;
   int k= 0;
   for(int v= 0; v < n; ++v)
    for(int u= v; dx[2 * u] == 1;) {
     const int p= dx[2 * u + 1];
     ord[k++]= u, par[u]= p, dx[2 * u]= 0;
     dx[2 * p + 1]^= u, --dx[2 * p];
     u= p;
    }
   ord[k]= root, par[root]= -1;
  } else {
   int *st= sz, *adj= hb;  // st は n + 1 個、adj は 2(n - 1) 個
   std::fill(st, st + n + 1, 0);
   for(const auto& e: edges) ++st[e[0] + 1], ++st[e[1] + 1];
   for(int i= 0; i < n; ++i) st[i + 1]+= st[i];
   for(const auto& e: edges) adj[st[e[0]]++]= e[1], adj[st[e[1]]++]= e[0];
   // st[v] は v の隣の終わり。始まりは st[v - 1] (v = 0 なら 0)。BFS の順を ord の後ろから前へ置く。
   int t= n - 1;
   ord[t]= root, par[root]= -1;
   for(int i= n - 1; i >= t; --i) {
    const int v= ord[i], pv= par[v];
    for(int j= v ? st[v - 1] : 0, e= st[v]; j < e; ++j)
     if(const int u= adj[j]; u != pv) par[u]= v, ord[--t]= u;
   }
  }
  std::fill(sz, sz + n, 1), std::fill(hb, hb + 2 * size_t(n), 0);
  for(int i= 0; i + 1 < n; ++i) {
   const int v= ord[i], p= par[v], s= sz[v];
   sz[p]+= s;
   const int c= -int(s > hb[2 * p]);
   hb[2 * p]= (s & c) | (hb[2 * p] & ~c), hb[2 * p + 1]= (v & c) | (hb[2 * p + 1] & ~c);
  }
  // 上から下へ位置を配る。sz[v] は v の位置を配ったあと、v の子を置く次の位置に書き換えて使い回す。
  int *pos= t.pos, *szp= t.szp, *upp= t.upp;
  u64* ch= t.ch;
  pos[root]= 0, szp[0]= n, upp[0]= 0, ch[0]= chain(0, 0);
  sz[root]= 1 + hb[2 * root];
  for(int i= n - 1; i--;) {
   const int v= ord[i], p= par[v], pp= pos[p], nx= sz[p], s= sz[v];
   const int h= -int(hb[2 * p + 1] == v);
   const int x= ((pp + 1) & h) | (nx & ~h);
   sz[p]= nx + (s & ~h);
   const u64 hm= 0 - u64(h & 1), cp= ch[pp];
   ch[x]= (cp & hm) | (chain(x, pp) & ~hm);
   pos[v]= x, szp[x]= s, upp[x]= pp;
   sz[v]= x + 1 + hb[2 * v];
  }
  return t;
 }
 // 位置 x と y の lca の位置。
 int lca_pos(int x, int y) const {
  u64 a= ch[x], b= ch[y];
  while(head_of(a) != head_of(b)) {
   if(head_of(a) > head_of(b)) x= up_of(a), a= ch[x];
   else y= up_of(b), b= ch[y];
  }
  return std::min(x, y);
 }
 // u から v への道を、u から順に位置の閉区間 [l, r] (l <= r) へ分ける。u から lca へ上る区間は up(l, r)
 // (位置 r から l へ向かう)、lca から v へ下りる区間は down(l, r) (位置 l から r へ向かう) を、道の順に呼ぶ。
 // v の側の区間は下から見つかるので、積んでおいて最後に逆順で呼ぶ。
 template <class Up, class Down> void path_ordered(int u, int v, const Up& up, const Down& down) const {
  int x= pos[u], y= pos[v];
  u64 a= ch[x], b= ch[y];
  int dl[64], dr[64], k= 0;
  while(head_of(a) != head_of(b)) {
   if(head_of(a) > head_of(b)) up(head_of(a), x), x= up_of(a), a= ch[x];
   else dl[k]= head_of(b), dr[k++]= y, y= up_of(b), b= ch[y];
  }
  if(x >= y) up(y, x);
  else down(x, y);
  while(k--) down(dl[k], dr[k]);
 }
};
}
