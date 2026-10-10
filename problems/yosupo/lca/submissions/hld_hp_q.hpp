#pragma once
// hld_hp の作る段 (分岐のまま) に、hld_hp_qb と同じ分岐の無いクエリを組み合わせたもの。作る段とクエリのどちらの
// 分岐が重いかを分けて見る。鎖の組は hld_hp_qb と同じく 1 つの u64 に詰める。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>
namespace lca_hld_hp_q {
using u64= unsigned long long;
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
// 鎖の組。下位 32 bit が鎖の先頭の位置、上位 32 bit が先頭の親の位置 (根の鎖は 0、使われない)。
inline u64 chain(int head, int up) { return u64(unsigned(head)) | u64(unsigned(up)) << 32; }
inline int head_of(u64 c) { return int(unsigned(c)); }
inline int up_of(u64 c) { return int(c >> 32); }
struct Tree {
 Buf buf;
 int *pos, *vert;
 u64* ch;
 Tree(int n, const std::vector<int>& par): buf(alloc_huge(6 * size_t(n))) {
  int *sz= buf.get(), *hv= sz + n;
  pos= hv + n, vert= pos + n, ch= (u64*)(vert + n);
  std::fill(sz, sz + n, 1), std::fill(hv, hv + n, -1);
  for(int i= n - 1; i > 0; --i) {
   const int p= par[i];
   sz[p]+= sz[i];
   if(hv[p] < 0 || sz[i] > sz[hv[p]]) hv[p]= i;
  }
  // 小さい順に位置を配る。sz[i] は i を配ったあと使わないので、i の子を置く次の位置 (nxt) に書き換えて使い回す。
  pos[0]= 0, vert[0]= 0, ch[0]= chain(0, 0);
  sz[0]= 1 + (hv[0] >= 0 ? sz[hv[0]] : 0);
  for(int i= 1; i < n; ++i) {
   const int p= par[i], pp= pos[p];
   int x;
   if(i == hv[p]) x= pp + 1, ch[x]= ch[pp];
   else x= sz[p], sz[p]+= sz[i], ch[x]= chain(x, pp);
   pos[i]= x, vert[x]= i;
   sz[i]= x + 1 + (hv[i] >= 0 ? sz[hv[i]] : 0);
  }
 }
 int lca(int u, int v) const {
  int x= pos[u], y= pos[v];
  u64 a= ch[x], b= ch[y];
  // 条件式で選ぶと GCC が上げる側で道を分けて分岐に戻すので、全ビットのマスクで選ぶ。
  while(head_of(a) != head_of(b)) {
   const u64 m= 0 - u64(head_of(a) > head_of(b));  // x の側を上げるなら全ビット
   const u64 z= ((a & m) | (b & ~m)) >> 32;
   const u64 c= ch[z];
   x= int((z & m) | (u64(unsigned(x)) & ~m)), y= int((u64(unsigned(y)) & m) | (z & ~m));
   a= (c & m) | (a & ~m), b= (b & m) | (c & ~m);
  }
  return vert[std::min(x, y)];
 }
};
}
struct Solver {
 lca_hld_hp_q::Tree t;
 Solver(int n, const vector<int>& par): t(n, par) {}
 int lca(int u, int v) const { return t.lca(u, v); }
};
