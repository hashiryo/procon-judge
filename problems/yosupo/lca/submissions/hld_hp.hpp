#pragma once
// HLD を、入力の保証 par[i] < i (親が子より前の番号) を使って、隣接リストを組まずに作るもの。番号の大きい順に
// 1 回走って部分木の大きさと重い子を決め、小さい順に 1 回走って、重い子を先にした行きがけ順の位置を配る。位置
// pos[p] の頂点 p の子は、重い子が pos[p] + 1、軽い子はその後ろに、部分木の大きさずつ詰めて置く。頂点を位置に
// 置き直し、位置ごとに、その鎖の先頭の位置と、先頭の親の位置を 8 byte の組で持つ。クエリは両端を位置に直し、鎖の
// 先頭の位置が大きいほうを先頭の親へ上げることを、先頭が揃うまで繰り返して、小さいほうの位置の頂点を返す。上げる
// たびに組を 1 つ読む。表は huge page に置く。一般の木で使うなら、先に BFS などで親が前に来る番号を作る必要がある。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>
namespace lca_hld_hp {
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
struct Tree {
 struct Chain {
  int head, up;  // 鎖の先頭の位置と、先頭の親の位置 (根の鎖は -1)
 };
 Buf buf;
 int *pos, *vert;
 Chain* ch;
 Tree(int n, const std::vector<int>& par): buf(alloc_huge(6 * size_t(n))) {
  int *sz= buf.get(), *hv= sz + n;
  pos= hv + n, vert= pos + n, ch= (Chain*)(vert + n);
  std::fill(sz, sz + n, 1), std::fill(hv, hv + n, -1);
  for(int i= n - 1; i > 0; --i) {
   const int p= par[i];
   sz[p]+= sz[i];
   if(hv[p] < 0 || sz[i] > sz[hv[p]]) hv[p]= i;
  }
  // 小さい順に位置を配る。sz[i] は i を配ったあと使わないので、i の子を置く次の位置 (nxt) に書き換えて使い回す。
  pos[0]= 0, vert[0]= 0, ch[0]= {0, -1};
  sz[0]= 1 + (hv[0] >= 0 ? sz[hv[0]] : 0);
  for(int i= 1; i < n; ++i) {
   const int p= par[i], pp= pos[p];
   int x;
   if(i == hv[p]) x= pp + 1, ch[x]= {ch[pp].head, ch[pp].up};
   else x= sz[p], sz[p]+= sz[i], ch[x]= {x, pp};
   pos[i]= x, vert[x]= i;
   sz[i]= x + 1 + (hv[i] >= 0 ? sz[hv[i]] : 0);
  }
 }
 int lca(int u, int v) const {
  int x= pos[u], y= pos[v];
  Chain a= ch[x], b= ch[y];
  while(a.head != b.head) {
   if(a.head > b.head) x= a.up, a= ch[x];
   else y= b.up, b= ch[y];
  }
  return vert[std::min(x, y)];
 }
};
}
struct Solver {
 lca_hld_hp::Tree t;
 Solver(int n, const vector<int>& par): t(n, par) {}
 int lca(int u, int v) const { return t.lca(u, v); }
};
