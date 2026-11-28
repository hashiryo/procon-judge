#pragma once
// 動的木の下書き。NeoLibrary に置く Link-Cut Tree の候補を、procon-judge の提出から使える形にしたもの。
//   LinkCutTree<M>: 道の積 (u から v への順、可換でなくてよい)。M は SegmentTree と同じモノイド
//     (using T、static T ti()、static T op(T, T))。作用 (using E、mp(T&, E)、cp(E&, E)) があれば道への作用も使える。
//   SubtreeLinkCutTree<M>: 可換なモノイドで、辺 (v, p) について p を親とした v の部分木の積を答える。
// 節は配列に置き、親は符号つきの int で持つ (正: splay の親、負: 道の親、0: なし)。回転では x の部分木が p の
// 前の部分木と同じ区間になるので、x の集約を p から写してから p だけ求め直す。Library Checker の
// dynamic_tree_vertex_add_path_sum の chaihf の 270626 と、dynamic_tree_vertex_set_path_composite の
// near1111 の 419210 の書き方を一般のモノイドにした (記録は algo-notes の notes/dynamic-tree.md)。
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>
#include "_shared/dyntree/common.hpp"

namespace dyntree_internal {
// 要る節だけを持つ。使わないときは空になる。空のメンバを同じ型で 3 個並べると、同じ型の部分オブジェクトは
// 同じ番地を持てないので、節が 40 byte に膨らむ。1 個の構造体にまとめて 32 byte に収める。
template <class M, bool NC, bool AC>
struct LctExtra {};
template <class M>
struct LctExtra<M, true, false> {
 typename M::T rsum = M::ti();
};
template <class M>
struct LctExtra<M, false, true> {
 typename ActionOf<M>::type laz{};
 bool lazf = false;
};
template <class M>
struct LctExtra<M, true, true> {
 typename M::T rsum = M::ti();
 typename ActionOf<M>::type laz{};
 bool lazf = false;
};
template <class M>
struct LctNode {
 using T = typename M::T;
 using E = typename ActionOf<M>::type;
 static constexpr bool NC = !Commutative<M>;
 static constexpr bool AC = Action<M>;
 int ch[2] = {0, 0};
 int par = 0;
 bool rev = false;
 T val = M::ti(), sum = M::ti();
 [[no_unique_address]] LctExtra<M, NC, AC> ex;
};
}  // namespace dyntree_internal

// 可換な群 (inv を持つ) のモノイドを受ける。道の積に加えて、辺 (v, p) で p を親とした v の部分木の積を答える。
// 節は splay の部分木の和 (道の上の頂点と、仮想の子の部分木) と、仮想の子の部分木の和 vir を持つ。access で
// 右の子を仮想へ移すとき vir に足し、仮の道の子を仮想から外すとき vir から引く ([[library-impl-single-call-latency]])。
template <class M>
class SubtreeLinkCutTree {
 using T = typename M::T;
 static constexpr bool CG = dyntree_internal::Commutative<M> && requires(T a) { M::inv(a); };
 static_assert(CG, "SubtreeLinkCutTree requires a commutative group (commute and inv)");
 struct Node {
  int ch[2] = {0, 0};
  int par = 0;
  bool rev = false;
  T val = M::ti(), sum = M::ti(), vir = M::ti();
 };
 dyntree_internal::Vec<Node> t;
 int n;
 [[gnu::always_inline]] void pull(int x) {
  Node& a = t[x];
  T s = a.val;
  if (int l = a.ch[0]; l) s = M::op(t[l].sum, s);
  s = M::op(s, a.vir);
  if (int r = a.ch[1]; r) s = M::op(s, t[r].sum);
  a.sum = s;
 }
 [[gnu::always_inline]] void reverse(int x) {
  if (!x) return;
  std::swap(t[x].ch[0], t[x].ch[1]), t[x].rev ^= 1;
 }
 [[gnu::always_inline]] void push(int x) {
  if (t[x].rev) reverse(t[x].ch[0]), reverse(t[x].ch[1]), t[x].rev = false;
 }
 [[gnu::always_inline]] void rotate(int x) {
  Node& a = t[x];
  int p = a.par, g = t[p].par, d = t[p].ch[1] == x, b = a.ch[d ^ 1];
  t[p].ch[d] = b;
  if (b) t[b].par = p;
  a.ch[d ^ 1] = p, t[p].par = x, a.par = g;
  if (g > 0) t[g].ch[t[g].ch[1] == p] = x;
  a.sum = t[p].sum;
  pull(p);
 }
 [[gnu::always_inline]] void splay(int x) {
  for (int p; (p = t[x].par) > 0;) {
   int g = t[p].par;
   if (g <= 0) {
    push(p), push(x), rotate(x);
    break;
   }
   push(g), push(p), push(x);
   if ((t[g].ch[1] == p) == (t[p].ch[1] == x)) rotate(p), rotate(x);
   else rotate(x), rotate(x);
  }
  push(x);
 }
 [[gnu::always_inline]] int access(int x) {
  splay(x);
  if (int r = t[x].ch[1]; r) t[r].par = -x, t[x].vir = M::op(t[x].vir, t[r].sum);
  t[x].ch[1] = 0;
  int last = x;
  while (int p = -t[x].par) {
   splay(p);
   if (int r = t[p].ch[1]; r) t[r].par = -p, t[p].vir = M::op(t[p].vir, t[r].sum);
   t[p].vir = M::op(t[p].vir, M::inv(t[x].sum));
   t[p].ch[1] = x;
   t[x].par = p;
   rotate(x);
   last = p;
  }
  pull(x);
  return last;
 }
 [[gnu::always_inline]] void evert(int x) { access(x), reverse(x); }

public:
 explicit SubtreeLinkCutTree(int n_): t(std::size_t(n_) + 2), n(n_) {}
 int size() const { return n; }
 T get(int v) {
  splay(v + 1);
  return t[v + 1].val;
 }
 // 1 点の値を d だけ動かす (群の op で x と d を合わせる)。
 void add(int v, T d) {
  int x = v + 1;
  access(x);
  t[x].val = M::op(t[x].val, d);
  pull(x);
 }
 void set(int v, T x) {
  int y = v + 1;
  access(y);
  t[y].val = x;
  pull(y);
 }
 void link(int u, int v) {
  u += 1, v += 1;
  evert(u), access(v);
  t[v].vir = M::op(t[v].vir, t[u].sum);
  t[u].par = -v;
  pull(v);
 }
 void cut(int u, int v) {
  u += 1, v += 1;
  evert(u), access(v);
  assert(t[v].ch[0] == u && t[u].ch[1] == 0);
  t[v].ch[0] = 0, t[u].par = 0;
  pull(v);
 }
 bool connected(int u, int v) { return root_(u + 1) == root_(v + 1); }
 int root(int v) { return root_(v + 1) - 1; }
 int parent(int v) {
  int r = parent_(v + 1);
  return r < 0 ? -1 : r - 1;
 }
 int lca(int u, int v) { return lca_(u + 1, v + 1) - 1; }
 // v の木の全体の積。
 T prod_tree(int v) {
  int x = v + 1;
  access(x);
  return t[x].sum;
 }
 // 辺 (v, p) について、p を親とした v の部分木の積。v と p は辺で結ばれていること。
 T prod_subtree(int v, int p) {
  int x = v + 1, y = p + 1;
  evert(x), access(y);
  return M::op(t[x].val, t[x].vir);
 }

 private:
 int root_(int x) {
  access(x);
  for (int y = x;;) {
   push(y);
   if (!t[y].ch[0]) return splay(y), y;
   y = t[y].ch[0];
  }
 }
 int parent_(int v) {
  access(v);
  if (!t[v].ch[0]) return -1;
  int y = t[v].ch[0];
  push(y);
  while (t[y].ch[1]) y = t[y].ch[1], push(y);
  splay(y);
  return y;
 }
 int lca_(int u, int v) {
  if (u == v) return u;
  access(u);
  return access(v);
 }
};

template <class M>
class LinkCutTree {
 using T = typename M::T;
 using E = typename dyntree_internal::ActionOf<M>::type;
 using Node = dyntree_internal::LctNode<M>;
 static constexpr bool NC = !dyntree_internal::Commutative<M>;
 static constexpr bool AC = dyntree_internal::Action<M>;
 dyntree_internal::Vec<Node> t;
 int n;
 [[gnu::always_inline]] void pull(int x) {
  Node& a = t[x];
  // 0 番の節は ti() なので、子の有無で分けずに op で畳める。
  a.sum = M::op(M::op(t[a.ch[0]].sum, a.val), t[a.ch[1]].sum);
  if constexpr (NC) a.ex.rsum = M::op(M::op(t[a.ch[1]].ex.rsum, a.val), t[a.ch[0]].ex.rsum);
 }
 [[gnu::always_inline]] void reverse(int x) {
  if constexpr (NC) std::swap(t[x].sum, t[x].ex.rsum);
  t[x].rev ^= 1;
 }
 template <bool A = AC>
 [[gnu::always_inline]] void apply_to(int x, const E& e) {
  if constexpr (A) {
   if (!x) return;
   Node& a = t[x];
   M::mp(a.val, e), M::mp(a.sum, e);
   if constexpr (NC) M::mp(a.ex.rsum, e);
   if (a.ex.lazf) M::cp(a.ex.laz, e);
   else a.ex.laz = e, a.ex.lazf = true;
  }
 }
 [[gnu::always_inline]] void push(int x) {
  Node& a = t[x];
  if (a.rev) {
   std::swap(a.ch[0], a.ch[1]);
   reverse(a.ch[0]), reverse(a.ch[1]);
   a.rev = false;
  }
  if constexpr (AC)
   if (a.ex.lazf) apply_to(a.ch[0], a.ex.laz), apply_to(a.ch[1], a.ex.laz), a.ex.lazf = false;
 }
 // x は splay の親を持つ。回転しても x の部分木の区間は p の前の部分木と同じなので、集約は p から写す。
 [[gnu::always_inline]] void rotate(int x) {
  Node& a = t[x];
  int p = a.par, g = t[p].par, d = t[p].ch[1] == x, b = a.ch[d ^ 1];
  t[p].ch[d] = b;
  t[a.ch[d ^ 1]].par = p;
  a.ch[d ^ 1] = p, t[p].par = x, a.par = g;
  if (g > 0) t[g].ch[t[g].ch[1] == p] = x;
  a.sum = t[p].sum;
  if constexpr (NC) a.ex.rsum = t[p].ex.rsum;
  pull(p);
 }
 [[gnu::always_inline]] void splay(int x) {
  for (int p; (p = t[x].par) > 0;) {
   int g = t[p].par;
   if (g <= 0) {
    push(p), push(x), rotate(x);
    break;
   }
   push(g), push(p), push(x);
   if ((t[g].ch[1] == p) == (t[p].ch[1] == x)) rotate(p), rotate(x);
   else rotate(x), rotate(x);
  }
  push(x);
 }
 [[gnu::always_inline]] int access(int x) {
  splay(x);
  t[t[x].ch[1]].par = -x;
  t[x].ch[1] = 0;
  int last = x;
  while (int p = -t[x].par) {
   splay(p);
   t[t[p].ch[1]].par = -p;
   t[p].ch[1] = x;
   t[x].par = p;
   rotate(x);
   last = p;
  }
  if constexpr (AC) pull(x);
  return last;
 }
 [[gnu::always_inline]] void evert(int x) { access(x), reverse(x); }

public:
 explicit LinkCutTree(int n_): t(std::size_t(n_) + 2), n(n_) {}
 int size() const { return n; }
 // 1 点の値。前の作用は splay で配ってから読む。
 T get(int v) {
  int x = v + 1;
  splay(x);
  return t[x].val;
 }
 // 道の作用が無いときの 1 点の代入。
 void set(int v, T x) {
  int y = v + 1;
  splay(y);
  t[y].val = x;
  pull(y);
 }
 // 1 点の値を op で動かす (可換でなくてよい)。
 void mul(int v, const T& d) {
  int y = v + 1;
  splay(y);
  t[y].val = M::op(t[y].val, d);
  if constexpr (dyntree_internal::Commutative<M>) t[y].sum = M::op(t[y].sum, d);
  else pull(y);
 }
 // 初期の値を置く (splay しない)。
 void init_value(int v, T x) {
  int y = v + 1;
  t[y].val = x, t[y].sum = x;
  if constexpr (NC) t[y].ex.rsum = x;
 }
 // 初期の木を親の向きで張る。すべて仮想辺なので splay しない (v の親 p)。
 void init_parent(int v, int p) { t[v + 1].par = -(p + 1); }
 // u と v を結ぶ (u と v は別の木にあること)。
 void link(int u, int v) {
  u += 1, v += 1;
  evert(u), t[u].par = -v;
 }
 // 辺 (u, v) を切る。仮想辺のときは、深い側の端が自分の道の根で、その道の親が浅い側の端になって
 // いるだけなので、道の親を消すだけでよい (回転も集約の計算も要らない)。道の上で隣り合っている
 // ときは、両端を splay すると深い側が浅い側の子になるので、そこで切り離す (near1111 の 419210)。
 // 道の親は浅い側に残す。
 void cut(int u, int v) {
  u += 1, v += 1;
  if (t[u].par == -v) {
   t[u].par = 0;
   return;
  }
  if (t[v].par == -u) {
   t[v].par = 0;
   return;
  }
  splay(u);
  if (t[u].par == -v) {
   t[u].par = 0;
   return;
  }
  splay(v);
  if (t[v].ch[0] == u) {
   t[u].par = t[v].par, t[v].par = 0, t[v].ch[0] = 0, pull(v);
  } else if (t[v].ch[1] == u) {
   t[u].par = 0, t[v].ch[1] = 0, pull(v);
  } else {
   assert(t[v].par == -u);
   t[v].par = 0;
  }
 }
 bool connected(int u, int v) { return root_(u + 1) == root_(v + 1); }
 int root(int v) { return root_(v + 1) - 1; }
 // v の親 (根なら -1)。
 int parent(int v) {
  int r = parent_(v + 1);
  return r < 0 ? -1 : r - 1;
 }
 // 同じ木の 2 頂点の lca。
 int lca(int u, int v) { return lca_(u + 1, v + 1) - 1; }
 // u から v への道の積 (道の順)。u と v は同じ木にあること。
 T prod(int u, int v) {
  u += 1, v += 1;
  evert(u), access(v);
  if constexpr (AC) return t[v].sum;
  return M::op(t[t[v].ch[0]].sum, t[v].val);
 }
 // 道 u-v への作用。
 template <bool A = AC>
 void apply(int u, int v, const E& e) {
  static_assert(A, "\"apply\" is not available");
  if constexpr (A) u += 1, v += 1, evert(u), access(v), apply_to(v, e);
 }

 private:
 int root_(int x) {
  access(x);
  for (int y = x;;) {
   push(y);
   if (!t[y].ch[0]) return splay(y), y;
   y = t[y].ch[0];
  }
 }
 int parent_(int v) {
  access(v);
  if (!t[v].ch[0]) return -1;
  int y = t[v].ch[0];
  push(y);
  while (t[y].ch[1]) y = t[y].ch[1], push(y);
  splay(y);
  return y;
 }
 int lca_(int u, int v) {
  if (u == v) return u;
  access(u);
  return access(v);
 }
};
