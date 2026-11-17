#pragma once
// 親の列から HLD を作る一般の形。hld_hp_b と違い p_i < i を仮定しない (どの頂点を根にした親の列でもよい)。
// 作り方を 2 つ書き、テンプレート引数で選ぶ。どちらも、子が親より前に来る順 (下から上の順) を作ってから、
// その順で部分木の大きさと重い子を決め、逆順で重い子を先にした行きがけ順の位置を配る。
//   MODE = 0 (peel): 子の数を数え、数が 0 になった頂点から親へ上がりながら並べる。子の隣接は組まない。
//   MODE = 1 (csr):  子の CSR を組み、根から BFS した順を逆にする。
//   MODE = 2 (auto): 先に p_i < i (根が 0) かを 1 回の走査で確かめ、そうなら番号の大きい順をそのまま使う。違えば peel。
// 問い合わせは hld_hp_b と同じく、位置ごとに鎖の先頭の位置と先頭の親の位置を 8 byte の組で持つ。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>
namespace lca_gen {
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
inline u64 chain(int head, int up) { return u64(unsigned(head)) | u64(unsigned(up)) << 32; }
inline int head_of(u64 c) { return int(unsigned(c)); }
inline int up_of(u64 c) { return int(c >> 32); }
template <int MODE> struct Tree {
 Buf buf, tmp;
 int *pos, *vert;
 u64* ch;
 Tree(int n, const std::vector<int>& par): buf(alloc_huge(4 * size_t(n))), tmp(alloc_huge(5 * size_t(n) + 1)) {
  pos= buf.get(), vert= pos + n, ch= (u64*)(vert + n);
  // hb[2v] = 子の部分木の大きさの最大、hb[2v + 1] = その子。sz は CSR の始まりの列 (n + 1 個) にも使うので 1 つ多く取る。
  int *ord= tmp.get(), *sz= ord + n, *hb= sz + n + 1;
  if(n <= 0) return;  // gcc が負の n で配列を埋める場合を疑って警告するので、先に返す
  bool ordered= false;
  if constexpr(MODE == 2) {
   // 分岐の無い and の積み上げで確かめる (ベクトル化される)。
   bool ok= n > 0 && par[0] < 0;
   for(int i= 1; i < n; ++i) ok&= unsigned(par[i]) < unsigned(i);
   if((ordered= ok))
    for(int i= 0; i < n; ++i) ord[i]= n - 1 - i;
  }
  if(ordered) {
  } else if constexpr(MODE != 1) {
   int* cnt= hb;  // 子の数。下から上の順を作り終えたら hb に使い直す
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
   int *st= sz, *ch_= hb;  // st は n + 1 個、ch_ は n 個。下から上の順を作り終えたら sz と hb に使い直す
   std::fill(st, st + n + 1, 0);
   int root= 0;
   for(int i= 0; i < n; ++i)
    if(par[i] >= 0) ++st[par[i] + 1];
    else root= i;
   for(int i= 0; i < n; ++i) st[i + 1]+= st[i];
   for(int i= 0; i < n; ++i)
    if(par[i] >= 0) ch_[st[par[i]]++]= i;
   // st[v] は v の子の終わりになった。始まりは st[v - 1] (v = 0 なら 0)。BFS の順を ord の後ろから前へ置くと、
   // ord を前から読んだ順が下から上の順になる。
   int t= n - 1;
   ord[t]= root;
   for(int i= n - 1; i >= t; --i) {
    const int v= ord[i];
    for(int j= v ? st[v - 1] : 0, e= st[v]; j < e; ++j) ord[--t]= ch_[j];
   }
  }
  std::fill(sz, sz + n, 1), std::fill(hb, hb + 2 * size_t(n), 0);
  for(int i= 0; i + 1 < n; ++i) {
   const int v= ord[i], p= par[v], s= sz[v];
   sz[p]+= s;
   const int c= -int(s > hb[2 * p]);
   hb[2 * p]= (s & c) | (hb[2 * p] & ~c), hb[2 * p + 1]= (v & c) | (hb[2 * p + 1] & ~c);
  }
  // 上から下へ位置を配る。sz[v] は v の位置を配ったあと、v の子を置く次の位置 (nxt) に書き換えて使い回す。
  const int root= ord[n - 1];
  pos[root]= 0, vert[0]= root, ch[0]= chain(0, 0);
  sz[root]= 1 + hb[2 * root];
  for(int i= n - 1; i--;) {
   const int v= ord[i], p= par[v], pp= pos[p], nx= sz[p];
   const int h= -int(hb[2 * p + 1] == v);  // v は p の子なので、hb[2p + 1] は p の本当の重い子
   const int x= ((pp + 1) & h) | (nx & ~h);
   sz[p]= nx + (sz[v] & ~h);
   const u64 hm= 0 - u64(h & 1), cp= ch[pp];
   ch[x]= (cp & hm) | (chain(x, pp) & ~hm);
   pos[v]= x, vert[x]= v;
   sz[v]= x + 1 + hb[2 * v];
  }
 }
 int lca(int u, int v) const {
  int x= pos[u], y= pos[v];
  u64 a= ch[x], b= ch[y];
  while(head_of(a) != head_of(b)) {
   if(head_of(a) > head_of(b)) x= up_of(a), a= ch[x];
   else y= up_of(b), b= ch[y];
  }
  return vert[std::min(x, y)];
 }
};
}
