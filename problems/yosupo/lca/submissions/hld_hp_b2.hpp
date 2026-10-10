#pragma once
// hld_hp_b のクエリで、毎回両側の次の鎖の組を先に読み込んでから、上げる側を分岐で選ぶもの。hld_hp_b の分岐の
// クエリは、上げる側の予想が当たれば、上げない側の組の読み込みを前の読み込みの到着を待たずに出せるが、外すと
// やり直しになる。両側を先に読めば、分岐より前に出した読み込みは予想を外しても捨てられず、どちらの側も待ちが
// 重なる。分岐の無い hld_hp_q は、次に読む場所が今読んだ組との比較で決まり、毎回読み込みの到着を待つので遅かった。
// 根の鎖の先頭の親は 0 にしてあるので、根の鎖の組からも読める (読んだ値は使わない)。作る段は hld_hp_b と同じ。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>
namespace lca_hld_hp_b2 {
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
 Tree(int n, const std::vector<int>& par): buf(alloc_huge(7 * size_t(n))) {
  int *sz= buf.get(), *hb= sz + n;  // hb[2v] = 子の部分木の大きさの最大、hb[2v + 1] = その子
  pos= hb + 2 * size_t(n), vert= pos + n, ch= (u64*)(vert + n);
  std::fill(sz, sz + n, 1), std::fill(hb, hb + 2 * size_t(n), 0);
  for(int i= n - 1; i > 0; --i) {
   const int p= par[i], s= sz[i];
   sz[p]+= s;
   // 条件式で選ぶと GCC が分岐に戻すので、全ビットのマスクで選ぶ。
   const int c= -int(s > hb[2 * p]);
   hb[2 * p]= (s & c) | (hb[2 * p] & ~c), hb[2 * p + 1]= (i & c) | (hb[2 * p + 1] & ~c);
  }
  // 小さい順に位置を配る。sz[i] は i を配ったあと使わないので、i の子を置く次の位置 (nxt) に書き換えて使い回す。
  pos[0]= 0, vert[0]= 0, ch[0]= chain(0, 0);
  sz[0]= 1 + hb[0];
  for(int i= 1; i < n; ++i) {
   const int p= par[i], pp= pos[p], nx= sz[p];
   const int h= -int(hb[2 * p + 1] == i);  // 重い子なら全ビット。子は 1 以上なので、重い子が無いときの 0 と取り違えない
   const int x= ((pp + 1) & h) | (nx & ~h);
   sz[p]= nx + (sz[i] & ~h);
   const u64 hm= 0 - u64(h & 1), cp= ch[pp];
   ch[x]= (cp & hm) | (chain(x, pp) & ~hm);
   pos[i]= x, vert[x]= i;
   sz[i]= x + 1 + hb[2 * i];
  }
 }
 int lca(int u, int v) const {
  int x= pos[u], y= pos[v];
  u64 a= ch[x], b= ch[y];
  while(head_of(a) != head_of(b)) {
   u64 ca= ch[up_of(a)], cb= ch[up_of(b)];
   // 使う側の分岐の中へ読み込みを押し込まれないよう、両方の値をここで出させる (GCC は片側を分岐の中へ移していた)。
   asm volatile("" : "+r"(ca), "+r"(cb));
   if(head_of(a) > head_of(b)) x= up_of(a), a= ca;
   else y= up_of(b), b= cb;
  }
  return vert[std::min(x, y)];
 }
};
}
struct Solver {
 lca_hld_hp_b2::Tree t;
 Solver(int n, const vector<int>& par): t(n, par) {}
 int lca(int u, int v) const { return t.lca(u, v); }
};
