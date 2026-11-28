#pragma once
// 動的木の下書き。NeoLibrary に置く Euler Tour Tree の候補。木を、頂点の節と辺ごとの 2 つの向きの節を並べた
// 巡回列で持ち、splay 木で分ける。辺 (v, p) について p を親とした v の部分木は、v->p の節の直後から p->v の
// 節の直前までの区間になる。部分木への作用はその区間に遅延で当てる。M はモノイド (T、ti、op) で、作用は
// 区間の頂点の数を受ける mp(T&, E, int) と cp(E&, E) を持つもの (部分木への加算のような使い方)。
// Library Checker の dynamic_tree_subtree_add_subtree_sum などのための試作 (記録は algo-notes の notes/dynamic-tree.md)。
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
#include "_shared/dyntree/common.hpp"

namespace dyntree_internal {
template <class M>
struct EttNode {
 using T = typename M::T;
 using E = typename ActionOf<M>::type;
 static constexpr bool AC = ActionLen<M>;
 int ch[2] = {0, 0};
 int par = 0;
 int cnt = 0;  // splay の部分木にある頂点の節の数
 T val = M::ti(), sum = M::ti();
 [[no_unique_address]] std::conditional_t<AC, E, Empty> laz;
 [[no_unique_address]] std::conditional_t<AC, bool, Empty> lazf;
 EttNode() {
  if constexpr (AC) lazf = false;
 }
};
}  // namespace dyntree_internal

template <class M>
class EulerTourTree {
 public:
  // a は u->v の向きの節、b は v->u の向きの節。
  struct Edge {
   int u, v, a, b;
  };

 private:
  using T = typename M::T;
  using E = typename dyntree_internal::ActionOf<M>::type;
  using Node = dyntree_internal::EttNode<M>;
  static constexpr bool AC = dyntree_internal::ActionLen<M>;
  dyntree_internal::Vec<Node> t;
  std::vector<int> free_nodes;
  dyntree_internal::OpenTable<Edge> es;
  int n;
  static unsigned long long key(int u, int v) {
   if (u > v) std::swap(u, v);
   return (unsigned long long)(unsigned)u << 32 | (unsigned)v;
  }
  [[gnu::always_inline]] void pull(int x) {
   Node& a = t[x];
   int l = a.ch[0], r = a.ch[1];
   a.cnt = (l ? t[l].cnt : 0) + (x <= n ? 1 : 0) + (r ? t[r].cnt : 0);
   T s = a.val;
   if (l) s = M::op(t[l].sum, s);
   if (r) s = M::op(s, t[r].sum);
   a.sum = s;
  }
  template <bool A = AC>
  [[gnu::always_inline]] void apply_to(int x, const E& e) {
   if constexpr (A) {
    if (!x) return;
    Node& a = t[x];
    if (x <= n) M::mp(a.val, e, 1);
    M::mp(a.sum, e, a.cnt);
    if (a.lazf) M::cp(a.laz, e);
    else a.laz = e, a.lazf = true;
   }
  }
  [[gnu::always_inline]] void push(int x) {
   if constexpr (AC)
    if (t[x].lazf) apply_to(t[x].ch[0], t[x].laz), apply_to(t[x].ch[1], t[x].laz), t[x].lazf = false;
  }
  [[gnu::always_inline]] void rotate(int x) {
   Node& a = t[x];
   int p = a.par, g = t[p].par, d = t[p].ch[1] == x, b = a.ch[d ^ 1];
   t[p].ch[d] = b;
   if (b) t[b].par = p;
   a.ch[d ^ 1] = p, t[p].par = x, a.par = g;
   if (g) t[g].ch[t[g].ch[1] == p] = x;
   a.cnt = t[p].cnt, a.sum = t[p].sum;
   pull(p);
  }
  [[gnu::always_inline]] void splay(int x) {
   for (int p; (p = t[x].par);) {
    int g = t[p].par;
    if (!g) {
     push(p), push(x), rotate(x);
     break;
    }
    push(g), push(p), push(x);
    if ((t[g].ch[1] == p) == (t[p].ch[1] == x)) rotate(p), rotate(x);
    else rotate(x), rotate(x);
   }
   push(x);
  }
  // x より前を切り離す。x の木は x から始まる。
  [[gnu::always_inline]] int split_before(int x) {
   splay(x);
   int l = t[x].ch[0];
   if (l) t[l].par = 0;
   t[x].ch[0] = 0;
   pull(x);
   return l;
  }
  [[gnu::always_inline]] int split_after(int x) {
   splay(x);
   int r = t[x].ch[1];
   if (r) t[r].par = 0;
   t[x].ch[1] = 0;
   pull(x);
   return r;
  }
  // a の列の後ろに b の列を継ぐ。a と b は別の木。
  [[gnu::always_inline]] int merge(int a, int b) {
   if (!a) return b;
   if (!b) return a;
   int x = a;
   push(x);
   while (t[x].ch[1]) x = t[x].ch[1], push(x);
   splay(x);
   t[x].ch[1] = b, t[b].par = x;
   pull(x);
   return x;
  }
  // 巡回列を x から始まる形に回す。
  [[gnu::always_inline]] int reroot(int x) {
   int l = split_before(x);
   return merge(x, l);
  }
  int root_of(int x) {
   splay(x);
   int y = x;
   push(y);
   while (t[y].ch[0]) y = t[y].ch[0], push(y);
   splay(y);
   return y;
  }
  [[gnu::always_inline]] void reset(int x) {
   Node& a = t[x];
   a.ch[0] = a.ch[1] = a.par = 0, a.cnt = 0, a.val = M::ti(), a.sum = M::ti();
   if constexpr (AC) a.lazf = false;
  }
  int alloc_edge_node() {
   if (!free_nodes.empty()) {
    int x = free_nodes.back();
    free_nodes.pop_back();
    reset(x);
    return x;
   }
   int x = int(t.size());
   t.emplace_back();
   reset(x);
   return x;
  }
  void free_edge_node(int x) {
   reset(x);
   free_nodes.push_back(x);
  }
  // 列の [l, r) から平衡な splay 木を組む。根を返す。
  int build_balanced(const std::vector<int>& seq, int l, int r) {
   if (l >= r) return 0;
   const int m = (l + r) >> 1, x = seq[m];
   const int L = build_balanced(seq, l, m), R = build_balanced(seq, m + 1, r);
   t[x].ch[0] = L, t[x].ch[1] = R;
   if (L) t[L].par = x;
   if (R) t[R].par = x;
   t[x].par = 0;
   pull(x);
   return x;
  }
  // v から p へ向かう節と p から v へ向かう節。辺は (u,v) の向きで覚えてある。
  std::pair<int, int> edge_nodes(int v, int p) const {
   const Edge* e = es.find(key(v, p));
   assert(e);
   if (e->u == v) return {e->a, e->b};
   return {e->b, e->a};
  }

 public:
 explicit EulerTourTree(int n_): t(std::size_t(3 * n_ + 8)), n(n_) {
  for (int v = 1; v <= n; ++v) t[v].cnt = 1;
 }
 // 辺の列 (森でもよい) と頂点の値から O(N) で組む。頂点の節は 1..N、辺の節は N+1 から。
 EulerTourTree(int n_, const std::vector<std::array<int, 2>>& edges, const std::vector<T>& a) {
  n = n_;
  const int E = int(edges.size()), base = n + 1;
  t.resize(std::size_t(n + 2 * E + 2));
  for (int v = 1; v <= n; ++v) t[v].cnt = 1, t[v].val = a[v - 1], t[v].sum = a[v - 1];
  std::vector<int> head(n, -1), to(2 * E), nxt(2 * E), eid(2 * E), dir(2 * E);
  int ec = 0;
  auto add_arc = [&](int u, int v, int e, int d) { to[ec] = v, nxt[ec] = head[u], eid[ec] = e, dir[ec] = d, head[u] = ec++; };
  for (int e = 0; e < E; ++e) {
   const int u = edges[e][0], v = edges[e][1];
   add_arc(u, v, e, 0), add_arc(v, u, e, 1);
   es.insert(key(u, v), Edge{u, v, base + 2 * e, base + 2 * e + 1});
  }
  std::vector<char> seen(n, 0);
  std::vector<int> seq;
  seq.reserve(std::size_t(n + 2 * E));
  std::vector<int> su, sp, se, snode;
  for (int root = 0; root < n; ++root) {
   if (seen[root]) continue;
   const int start = int(seq.size());
   seen[root] = 1, seq.push_back(root + 1);
   su.push_back(root), sp.push_back(-1), se.push_back(head[root]), snode.push_back(-1);
   while (!su.empty()) {
    const int u = su.back(), p = sp.back();
    int& e = se.back();
    int v = -1, ee = -1, dd = -1;
    while (e != -1) {
     const int idx = e;
     e = nxt[idx];
     if (to[idx] != p) {
      v = to[idx], ee = eid[idx], dd = dir[idx];
      break;
     }
    }
    if (v == -1) {
     if (const int in = snode.back(); in != -1) seq.push_back(in);
     su.pop_back(), sp.pop_back(), se.pop_back(), snode.pop_back();
    } else {
     seen[v] = 1;
     const int node = base + 2 * ee + dd;
     seq.push_back(node), seq.push_back(v + 1);
     su.push_back(v), sp.push_back(u), se.push_back(head[v]), snode.push_back(base + 2 * ee + (dd ^ 1));
    }
   }
   build_balanced(seq, start, int(seq.size()));
  }
 }
 int size() const { return n; }
 bool edge_exists(int u, int v) const { return es.contains(key(u, v)); }
 T get(int v) {
  splay(v + 1);
  return t[v + 1].val;
 }
 void set(int v, T x) {
  int y = v + 1;
  splay(y);
  t[y].val = x;
  pull(y);
 }
 void link(int u, int v) {
  assert(u != v && !edge_exists(u, v));
  int a = alloc_edge_node(), b = alloc_edge_node();
  int ru = reroot(u + 1), rv = reroot(v + 1);
  int x = merge(merge(merge(ru, a), rv), b);
  static_cast<void>(x);
  es.insert(key(u, v), Edge{u, v, a, b});
 }
 void cut(int u, int v) {
  const Edge* found = es.find(key(u, v));
  assert(found);
  Edge e = *found;
  es.erase(key(u, v));
  int a, b;
  if (e.u == u) a = e.a, b = e.b;
  else a = e.b, b = e.a;
  reroot(a);  // 列を a から始め、a の次に v の側、その次に b が来る形にする
  int rest = split_after(a);
  static_cast<void>(rest);
  splay(b);
  int mid = split_before(b);  // v の側
  int suf = split_after(b);   // u の側 (a より前の部分は回したので空)
  static_cast<void>(mid);
  free_edge_node(a), free_edge_node(b);
  static_cast<void>(suf);
 }
 bool connected(int u, int v) { return root_of(u + 1) == root_of(v + 1); }
 // 頂点 v の木の全体の積。
 T prod_tree(int v) {
  int x = v + 1;
  splay(x);
  return t[x].sum;
 }
 // 木の全体の頂点の数。
 int tree_size(int v) {
  int x = v + 1;
  splay(x);
  return t[x].cnt;
 }
 // 木の全体への作用。
 template <bool A = AC>
 void apply_tree(int v, const E& e) {
  static_assert(A, "\"apply_tree\" is not available");
  if constexpr (A) splay(v + 1), apply_to(v + 1, e);
 }
 // 辺 (v, p) について、p を親とした v の部分木の積。v と p は辺で結ばれていること。
 T prod_subtree(int v, int p) {
  auto [a, b] = edge_nodes(v, p);  // a: v->p, b: p->v
  reroot(b), splay(a);             // p->v から前方が v の側
  int l = t[a].ch[0];
  return l ? t[l].sum : M::ti();
 }
 // 辺 (v, p) について、p を親とした v の部分木への作用。
 template <bool A = AC>
 void apply_subtree(int v, int p, const E& e) {
  static_assert(A, "\"apply_subtree\" is not available");
  if constexpr (A) {
   auto [a, b] = edge_nodes(v, p);  // a: v->p, b: p->v
   reroot(b), splay(a);
   apply_to(t[a].ch[0], e);
   pull(a);
  }
 }
};
