#pragma once
#include "pj.hpp"
// 部分木が区間になる位置を配って、区間和の木を載せる。位置の配り方を 3 つ書き、テンプレート引数で選ぶ。
//   Ord::Index (p_i < i を使う): 番号の大きい順に部分木の大きさを足し、小さい順に親の次の位置から子へ区間を配る。
//                                HLD はしない (Library Checker の首位と同じ形)。
//   Ord::Peel  (一般の木): 子の数を数えて葉から剥がした順で、部分木の大きさと重い子を決め、逆順で重い子を先にした
//                          行きがけ順の位置を配る (HLD の位置)。lca の gen_peel と同じ作り方。
//   Ord::Csr   (一般の木): 子の CSR を組んで根から BFS した順で、Peel と同じ位置を配る。
//   Ord::Auto  (一般の木): 先に p_i < i (根が 0) かを確かめ、そうなら番号の大きい順をそのまま使って Peel と同じ位置を配る。
//                          違えば Peel。
// 頂点ごとに位置と部分木の終わりの組を 8 byte で持ち、問い合わせで 1 回だけ読む。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
namespace subtree_ord {
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
enum class Ord { Index, Peel, Csr, Auto };
// pe[2v] = v の位置、pe[2v + 1] = v の部分木の終わり (半開区間)。
template <Ord O> inline Buf layout(int n, const vector<int>& par) {
 Buf pe= alloc_huge(2 * size_t(n));
 int* const P= pe.get();
 if(n <= 0) return pe;  // gcc が負の n で配列を埋める場合を疑って警告するので、先に返す
 if constexpr(O == Ord::Index) {
  Buf tmp= alloc_huge(size_t(n));
  int* sz= tmp.get();
  std::fill(sz, sz + n, 1);
  for(int i= n - 1; i > 0; --i) sz[par[i]]+= sz[i];
  // sz[v] は v の位置を配ったあと、v の子を置く次の位置に書き換えて使い回す。
  P[0]= 0, P[1]= n, sz[0]= 1;
  for(int i= 1; i < n; ++i) {
   const int p= par[i], x= sz[p], s= sz[i];
   sz[p]= x + s, P[2 * i]= x, P[2 * i + 1]= x + s, sz[i]= x + 1;
  }
  return pe;
 } else {
  Buf tmp= alloc_huge(5 * size_t(n) + 1);
  int *ord= tmp.get(), *sz= ord + n, *hb= sz + n + 1;  // hb[2v] = 子の部分木の大きさの最大、hb[2v + 1] = その子
  bool ordered= false;
  if constexpr(O == Ord::Auto) {
   bool ok= par[0] < 0;
   for(int i= 1; i < n; ++i) ok&= unsigned(par[i]) < unsigned(i);
   if((ordered= ok))
    for(int i= 0; i < n; ++i) ord[i]= n - 1 - i;
  }
  if(ordered) {
  } else if constexpr(O != Ord::Csr) {
   int* cnt= hb;
   std::fill(cnt, cnt + n, 0);
   for(int i= 0; i < n; ++i)
    if(par[i] >= 0) ++cnt[par[i]];
   int k= 0;
   for(int v= 0; v < n; ++v)
    for(int u= v; cnt[u] == 0;) {
     ord[k++]= u, cnt[u]= -1;
     const int p= par[u];
     if(p < 0 || --cnt[p] != 0) break;
     u= p;
    }
  } else {
   int *st= sz, *cs= hb;
   std::fill(st, st + n + 1, 0);
   int root= 0;
   for(int i= 0; i < n; ++i)
    if(par[i] >= 0) ++st[par[i] + 1];
    else root= i;
   for(int i= 0; i < n; ++i) st[i + 1]+= st[i];
   for(int i= 0; i < n; ++i)
    if(par[i] >= 0) cs[st[par[i]]++]= i;
   int t= n - 1;
   ord[t]= root;
   for(int i= n - 1; i >= t; --i) {
    const int v= ord[i];
    for(int j= v ? st[v - 1] : 0, e= st[v]; j < e; ++j) ord[--t]= cs[j];
   }
  }
  std::fill(sz, sz + n, 1), std::fill(hb, hb + 2 * size_t(n), 0);
  for(int i= 0; i + 1 < n; ++i) {
   const int v= ord[i], p= par[v], s= sz[v];
   sz[p]+= s;
   const int c= -int(s > hb[2 * p]);
   hb[2 * p]= (s & c) | (hb[2 * p] & ~c), hb[2 * p + 1]= (v & c) | (hb[2 * p + 1] & ~c);
  }
  const int root= ord[n - 1];
  P[2 * root]= 0, P[2 * root + 1]= n;
  sz[root]= 1 + hb[2 * root];
  for(int i= n - 1; i--;) {
   const int v= ord[i], p= par[v], pp= P[2 * p], nx= sz[p], s= sz[v];
   const int h= -int(hb[2 * p + 1] == v);
   const int x= ((pp + 1) & h) | (nx & ~h);
   sz[p]= nx + (s & ~h);
   P[2 * v]= x, P[2 * v + 1]= x + s;
   sz[v]= x + 1 + hb[2 * v];
  }
  return pe;
 }
}
// 位置に置いた値の BIT。1 点の加算と、[l, r) の和。
struct Fenwick {
 int n;
 Buf raw;
 i64* b;
 Fenwick(int n, const int* pe, const vector<i64>& a): n(n), raw(alloc_huge(2 * size_t(n) + 2)), b((i64*)raw.get()) {
  b[0]= 0;
  for(int v= 0; v < n; ++v) b[pe[2 * v] + 1]= a[v];
  for(int i= 1; i <= n; ++i)
   if(const int j= i + (i & -i); j <= n) b[j]+= b[i];
 }
 void add(int i, i64 x) {
  for(++i; i <= n; i+= i & -i) b[i]+= x;
 }
 i64 sum(int l, int r) const {
  i64 s= 0;
  for(; r > l; r&= r - 1) s+= b[r];
  for(; l > r; l&= l - 1) s-= b[l];
  return s;
 }
};
}
