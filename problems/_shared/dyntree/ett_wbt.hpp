#pragma once
// 動的木の下書き。Euler Tour Tree を splay 木ではなく、大きさで釣り合いを取る木 (WBT) で持つ版
// (比較用)。優先度を持たず、継ぐときは大きい側の木へ降りる。
// 列は、頂点の節と辺ごとの 2 つの向きの節を並べた巡回列。区間の和と区間への作用は、木を組み替えずに
// 範囲に降りて求める (splay 版は splay で区間を作る)。連結と切り離しだけが分割と連結になる。
// 比較の記録は algo-notes の notes/dynamic-tree.md。
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
#include "_shared/dyntree/common.hpp"

namespace ett_wbt_internal {
template <class M, bool AC>
struct Extra {};
template <class M>
struct Extra<M, true> {
 typename dyntree_internal::ActionOf<M>::type laz{};
 bool laz_f = false;
};
template <class M>
struct Node {
 using T = typename M::T;
 using E = typename dyntree_internal::ActionOf<M>::type;
 static constexpr bool AC = dyntree_internal::ActionLen<M>;
 T val = M::ti(), sum = M::ti();
 int l = 0, r = 0, par = 0;
 int size = 0;  // 部分木の要素の数
 int cnt = 0;   // 部分木の頂点の節の数
 [[no_unique_address]] Extra<M, AC> ex;
};
}  // namespace ett_wbt_internal

template <class M>
class WbtEulerTourTree {
 public:
  // a は u->v の向きの節、b は v->u の向きの節。
  struct Edge {
   int u, v, a, b;
  };

 private:
  using T = typename M::T;
  using E = typename dyntree_internal::ActionOf<M>::type;
  using Node = ett_wbt_internal::Node<M>;
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
   a.size = t[a.l].size + 1 + t[a.r].size;
   a.cnt = t[a.l].cnt + (x <= n ? 1 : 0) + t[a.r].cnt;
   a.sum = M::op(M::op(t[a.l].sum, a.val), t[a.r].sum);
  }
  template <bool A = AC>
  [[gnu::always_inline]] void apply_to(int x, const E& e) {
   if constexpr (A) {
    if (!x) return;
    Node& a = t[x];
    if (x <= n) M::mp(a.val, e, 1);
    M::mp(a.sum, e, a.cnt);
    if (a.ex.laz_f) M::cp(a.ex.laz, e);
    else a.ex.laz = e, a.ex.laz_f = true;
   }
  }
  [[gnu::always_inline]] void push(int x) {
   if constexpr (AC)
    if (t[x].ex.laz_f) apply_to(t[x].l, t[x].ex.laz), apply_to(t[x].r, t[x].ex.laz), t[x].ex.laz_f = false;
  }
  // 根まで登って、上から遅延を配る。
  void push_path(int x) {
   int st[64], k = 0;
   for (int y = x; y && k < 64; y = t[y].par) st[k++] = y;
   while (k) push(st[--k]);
  }
  // 二つの列を継ぐ。a と b はそれぞれの根。
  int merge(int a, int b) {
   if (!a) return b;
   if (!b) return a;
   if (t[a].size > t[b].size) {
    push(a);
    int r = merge(t[a].r, b);
    t[a].r = r, t[r].par = a;
    pull(a);
    return a;
   }
   push(b);
   int l = merge(a, t[b].l);
   t[b].l = l, t[l].par = b;
   pull(b);
   return b;
  }
  // 列 x を、先頭の k 個と残りに分ける。
  void split(int x, int k, int& a, int& b) {
   if (!x) return a = 0, b = 0, void();
   push(x);
   const int ls = t[t[x].l].size;
   if (k <= ls) {
    split(t[x].l, k, a, t[x].l);
    if (t[x].l) t[t[x].l].par = x;
    t[x].par = 0;
    b = x;
    pull(x);
   } else {
    split(t[x].r, k - ls - 1, t[x].r, b);
    if (t[x].r) t[t[x].r].par = x;
    t[x].par = 0;
    a = x;
    pull(x);
   }
  }
  int rank(int x) const {
   int r = t[t[x].l].size;
   for (int y = x; t[y].par;) {
    const int p = t[y].par;
    if (t[p].r == y) r += t[t[p].l].size + 1;
    y = p;
   }
   return r;
  }
  int root_of(int x) const {
   while (t[x].par) x = t[x].par;
   return x;
  }
  // 部分木 x の中の位置 [l, r) の和。
  T range_sum(int x, int l, int r) {
   if (!x || l >= r) return M::ti();
   if (l == 0 && r == t[x].size) return t[x].sum;
   push(x);
   const int ls = t[t[x].l].size;
   T s = M::ti();
   if (l < ls) s = range_sum(t[x].l, l, std::min(r, ls));
   if (l <= ls && ls < r) s = M::op(s, t[x].val);
   if (r > ls + 1) s = M::op(s, range_sum(t[x].r, std::max(l, ls + 1) - ls - 1, r - ls - 1));
   return s;
  }
  // 部分木 x の中の位置 [l, r) に作用を当てる。
  template <bool A = AC>
  void range_apply(int x, int l, int r, const E& e) {
   if constexpr (A) {
    if (!x || l >= r) return;
    if (l == 0 && r == t[x].size) return apply_to(x, e), void();
    push(x);
    const int ls = t[t[x].l].size;
    if (l < ls) range_apply(t[x].l, l, std::min(r, ls), e);
    if (l <= ls && ls < r)
     if (x <= n) M::mp(t[x].val, e, 1);
    if (r > ls + 1) range_apply(t[x].r, std::max(l, ls + 1) - ls - 1, r - ls - 1, e);
    pull(x);
   }
  }
  // x を先頭に回した木の根を返す。
  int reroot(int x) {
   const int root = root_of(x), k = rank(x);
   int a, b;
   split(root, k, a, b);
   int m = merge(b, a);
   t[m].par = 0;
   return m;
  }
  // v から p へ向かう節と p から v へ向かう節。辺は (u,v) の向きで覚えてある。
  std::pair<int, int> edge_nodes(int v, int p) const {
   const Edge* found = es.find(key(v, p));
   assert(found);
   if (found->u == v) return {found->a, found->b};
   return {found->b, found->a};
  }
  void reset(int x) {
   Node& a = t[x];
   a.l = a.r = a.par = 0, a.size = 1, a.cnt = x <= n ? 1 : 0;
   a.val = M::ti(), a.sum = a.val;
   if constexpr (AC) a.ex.laz_f = false;
  }
  int alloc_edge_node() {
   if (!free_nodes.empty()) {
    const int x = free_nodes.back();
    free_nodes.pop_back();
    reset(x);
    return x;
   }
   const int x = int(t.size());
   t.emplace_back();
   reset(x);
   return x;
  }
  void free_edge_node(int x) {
   reset(x);
   free_nodes.push_back(x);
  }
  // 列 seq から O(N) で釣り合った木を組む (区間の中央を根にする)。
  int build_range(const std::vector<int>& seq, int l, int r) {
   if (l >= r) return 0;
   const int m = (l + r) >> 1, x = seq[m];
   const int L = build_range(seq, l, m), R = build_range(seq, m + 1, r);
   t[x].l = L, t[x].r = R;
   if (L) t[L].par = x;
   if (R) t[R].par = x;
   t[x].par = 0;
   pull(x);
   return x;
  }
  int build(const std::vector<int>& seq) { return build_range(seq, 0, int(seq.size())); }

 public:
  explicit WbtEulerTourTree(int n_): t(std::size_t(3 * n_ + 8)), n(n_) {
   for (int v = 1; v <= n; ++v) reset(v), t[v].val = M::ti(), t[v].sum = M::ti();
  }
  // 辺の列 (森でもよい) と頂点の値から O(N) で組む。
  WbtEulerTourTree(int n_, const std::vector<std::array<int, 2>>& edges, const std::vector<T>& a) {
   n = n_;
   const int E = int(edges.size()), base = n + 1;
   t.resize(std::size_t(n + 2 * E + 2));
   for (int v = 1; v <= n; ++v) reset(v), t[v].val = a[v - 1], t[v].sum = a[v - 1];
   std::vector<int> head(n, -1), to(2 * E), nxt(2 * E), eid(2 * E), dir(2 * E);
   int ec = 0;
   auto add_arc = [&](int u, int v, int e, int d) { to[ec] = v, nxt[ec] = head[u], eid[ec] = e, dir[ec] = d, head[u] = ec++; };
   for (int e = 0; e < E; ++e) {
    const int u = edges[e][0], v = edges[e][1];
    add_arc(u, v, e, 0), add_arc(v, u, e, 1);
    es.insert(key(u, v), Edge{u, v, base + 2 * e, base + 2 * e + 1});
   }
   std::vector<char> seen(n, 0);
   std::vector<int> seq, su, sp, se, snode;
   seq.reserve(std::size_t(n + 2 * E));
   for (int root = 0; root < n; ++root) {
    if (seen[root]) continue;
    reset(root + 1);
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
      reset(node);
      seq.push_back(node), seq.push_back(v + 1);
      su.push_back(v), sp.push_back(u), se.push_back(head[v]), snode.push_back(base + 2 * ee + (dd ^ 1));
     }
    }
   }
   // 頂点の値を入れ直す (reset で消えるため)
   for (int v = 1; v <= n; ++v) t[v].val = a[v - 1];
   build(seq);
  }
  int size() const { return n; }
  bool edge_exists(int u, int v) const { return es.contains(key(u, v)); }
  T get(int v) {
   const int x = v + 1;
   push_path(x);
   return t[x].val;
  }
  void set(int v, T x) {
   const int y = v + 1;
   push_path(y);
   t[y].val = x;
   for (int z = y; z; z = t[z].par) pull(z);
  }
  void link(int u, int v) {
   assert(u != v && !edge_exists(u, v));
   const int a = alloc_edge_node(), b = alloc_edge_node();
   const int ru = reroot(u + 1), rv = reroot(v + 1);
   const int x = merge(merge(merge(ru, a), rv), b);
   t[x].par = 0;
   es.insert(key(u, v), Edge{u, v, a, b});
  }
  void cut(int u, int v) {
   const Edge* found = es.find(key(u, v));
   assert(found);
   const Edge e = *found;
   es.erase(key(u, v));
   int a, b;
   if (e.u == u) a = e.a, b = e.b;
   else a = e.b, b = e.a;
   const int root = root_of(a);
   int ra = rank(a), rb = rank(b);
   int p1, p2, p3, p4, p5, mid;
   if (ra > rb) {  // [.. b .. v の側 .. a ..]
    split(root, rb, p1, p2);       // p1: u の側の前, p2: b から
    split(p2, 1, mid, p3);         // mid = b
    split(p3, ra - rb - 1, mid, p4);  // mid = v の側
    split(p4, 1, mid, p5);         // mid = a
    static_cast<void>(merge(p5, p1));
   } else {  // [.. a .. u の側 .. b ..] で、v の側は折り返し
    split(root, ra, p1, p2);
    split(p2, 1, mid, p3);         // mid = a
    split(p3, rb - ra - 1, mid, p4);  // mid = u の側
    split(p4, 1, mid, p5);         // mid = b
    static_cast<void>(merge(p5, p1));
   }
   free_edge_node(a), free_edge_node(b);
  }
  bool connected(int u, int v) { return root_of(u + 1) == root_of(v + 1); }
  // 頂点 v の木の全体の積。
  T prod_tree(int v) {
   const int x = root_of(v + 1);
   return t[x].sum;
  }
  // 木の全体の頂点の数。
  int tree_size(int v) {
   const int x = root_of(v + 1);
   return t[x].cnt;
  }
  // 木の全体への作用。
  template <bool A = AC>
  void apply_tree(int v, const E& e) {
   static_assert(A, "\"apply_tree\" is not available");
   if constexpr (A) {
    const int x = root_of(v + 1);
    apply_to(x, e);
   }
  }
  // 辺 (v, p) について、p を親とした v の部分木の積。
  T prod_subtree(int v, int p) {
   auto [a, b] = edge_nodes(v, p);  // a: v->p, b: p->v
   const int root = root_of(b);
   const int ra = rank(a), rb = rank(b);
   if (ra > rb) return range_sum(root, rb + 1, ra);
   T s = range_sum(root, rb + 1, t[root].size);
   return M::op(s, range_sum(root, 0, ra));
  }
  // 辺 (v, p) について、p を親とした v の部分木への作用。
  template <bool A = AC>
  void apply_subtree(int v, int p, const E& e) {
   static_assert(A, "\"apply_subtree\" is not available");
   if constexpr (A) {
    auto [a, b] = edge_nodes(v, p);  // a: v->p, b: p->v
    const int root = root_of(b);
    const int ra = rank(a), rb = rank(b);
    if (ra > rb) range_apply(root, rb + 1, ra, e);
    else range_apply(root, rb + 1, t[root].size, e), range_apply(root, 0, ra, e);
   }
  }
};
