#pragma once
// 永続セグメント木。wavelet matrix と比べる相手として、yosupo の range-kth-smallest、static-range-sum-with-upper-bound、
// rectangle-sum の pst が共有する。値の順位 [0, 2^lg) を葉に持つ完全二分木で、節点に部分木の個数 (Sum なら重みの和も)
// を持つ。列の先頭から 1 個ずつ、根から葉への道を写しながら入れ、版 i を先頭 i 個を入れた木にする。[l, r) の問い合わせは
// 版 r と版 l を同時に下りて差を取る。節点は 1 本の配列に取り (2 MB 以上は huge page を頼む)、0 番を空の木 (子が自分) に
// する。下りる側の選び方は分岐なし。設計は algo-notes の notes/wavelet-matrix.md。
#include <type_traits>
#include "wm.hpp"
namespace wavelet {
template <bool Sum= false> struct PersistentSegTree {
 struct NodeC {
  int l, r, c;
 };
 struct NodeS {
  int l, r, c;
  i64 s;
 };
 using Node= std::conditional_t<Sum, NodeS, NodeC>;
 int lg= 1;
 u32 sigma= 0;
 std::size_t cap= 0;
 Node* t= nullptr;
 std::vector<int> root;
 PersistentSegTree(const PersistentSegTree&)= delete;
 PersistentSegTree& operator=(const PersistentSegTree&)= delete;
 // v は 0 以上 sigma 未満。Sum のときは w に重みを渡す。
 PersistentSegTree(const std::vector<u32>& v, u32 sigma_, const std::vector<i64>* w= nullptr): sigma(sigma_) {
  while((u32(1) << lg) < sigma) ++lg;
  const int n= v.size();
  cap= std::size_t(n) * (lg + 1) + 1;
  t= HugeAlloc<Node>().allocate(cap);
  t[0]= Node{};
  root.assign(n + 1, 0);
  int cnt= 1;
  for(int i= 0; i < n; ++i) {
   int prev= root[i], cur= cnt++;
   root[i + 1]= cur;
   for(int h= lg - 1; h >= 0; --h) {
    t[cur]= t[prev], ++t[cur].c;
    if constexpr(Sum) t[cur].s+= (*w)[i];
    const int nx= cnt++;
    if(v[i] >> h & 1) t[cur].r= nx, prev= t[prev].r;
    else t[cur].l= nx, prev= t[prev].l;
    cur= nx;
   }
   t[cur]= t[prev], ++t[cur].c;
   if constexpr(Sum) t[cur].s+= (*w)[i];
  }
 }
 ~PersistentSegTree() {
  if(t) HugeAlloc<Node>().deallocate(t, cap);
 }
 // [l, r) の小さい方から k 番目の値の順位。0 <= k < r - l。
 u32 kth(int l, int r, int k) const {
  int a= root[l], b= root[r];
  u32 res= 0;
  for(int h= lg - 1; h >= 0; --h) {
   const int cl= t[t[b].l].c - t[t[a].l].c, m= mask(k >= cl);
   k-= cl & m;
   res|= u32(m & 1) << h;
   a= sel(m, t[a].r, t[a].l);
   b= sel(m, t[b].r, t[b].l);
  }
  return res;
 }
 // [l, r) の順位 x 未満の値の数と重みの和。x は 0 以上 sigma 以下。Sum のときだけ。
 std::pair<int, i64> count_sum_less(int l, int r, u32 x) const {
  static_assert(Sum);
  int a= root[l], b= root[r];
  if(x >= sigma) return {t[b].c - t[a].c, t[b].s - t[a].s};
  int c= 0;
  i64 s= 0;
  for(int h= lg - 1; h >= 0; --h) {
   const int m= mask(x >> h & 1);
   const Node &la= t[t[a].l], &lb= t[t[b].l];
   c+= (lb.c - la.c) & m, s+= (lb.s - la.s) & i64(m);
   a= sel(m, t[a].r, t[a].l);
   b= sel(m, t[b].r, t[b].l);
  }
  return {c, s};
 }
};
}
