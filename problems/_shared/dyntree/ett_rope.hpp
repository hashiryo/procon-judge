#pragma once
// 動的木の下書き。Euler Tour Tree を、要素を 1 個ずつ持つ木ではなく、要素の ID を最大 CAP 個持つ
// チャンクの木 (ロープ) で持つ版 (splay 版との比較用)。節の数が 1/CAP になるので、木をたどる間の
// メモリがキャッシュに載りやすい。区間の和と作用は、木を組み替えずに範囲へ降りて求める。
// 記録は algo-notes の notes/dynamic-tree.md。
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
#include "_shared/dyntree/common.hpp"

namespace rope_ett_internal {
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
 static constexpr int CAP = 32;
 int ch[2] = {0, 0}, par = 0;
 int len = 0;           // このチャンクの要素数
 int tot = 0, cnt = 0;  // 部分木の要素数と頂点の数
 int mycnt = 0;         // このチャンクの頂点の数
 T mysum = M::ti();     // このチャンクの和
 T sum = M::ti();       // 部分木の和
 int ids[CAP] = {};
 [[no_unique_address]] Extra<M, AC> ex;
};
}  // namespace rope_ett_internal

template <class M>
class RopeEulerTourTree {
 public:
  // a は u->v の向きの節、b は v->u の向きの節。
  struct Edge {
   int u, v, a, b;
  };

 private:
  using T = typename M::T;
  using E = typename dyntree_internal::ActionOf<M>::type;
  using Node = rope_ett_internal::Node<M>;
  static constexpr bool AC = dyntree_internal::ActionLen<M>;
  static constexpr int CAP = Node::CAP;
  dyntree_internal::Vec<Node> t;
  dyntree_internal::Vec<T> val;  // 要素の値 (添字は要素の ID)
  std::vector<int> cid, slot;    // 要素の ID → チャンクと位置
  std::vector<int> free_chunks;
  dyntree_internal::OpenTable<Edge> es;
  int n;
  static unsigned long long key(int u, int v) {
   if (u > v) std::swap(u, v);
   return (unsigned long long)(unsigned)u << 32 | (unsigned)v;
  }
  [[gnu::always_inline]] void pull(int c) {
   const Node& a = t[c];
   Node& w = t[c];
   w.tot = t[a.ch[0]].tot + a.len + t[a.ch[1]].tot;
   w.cnt = t[a.ch[0]].cnt + a.mycnt + t[a.ch[1]].cnt;
   w.sum = M::op(M::op(t[a.ch[0]].sum, a.mysum), t[a.ch[1]].sum);
  }
  // 部分木への作用。このチャンクの要素にはその場で当て、子へは遅延で渡す。
  template <bool A = AC>
  [[gnu::always_inline]] void apply_to(int c, const E& e) {
   if constexpr (A) {
    if (!c) return;
    Node& a = t[c];
    for (int i = 0; i < a.len; ++i)
     if (const int id = a.ids[i]; id <= n) M::mp(val[id], e, 1);
    M::mp(a.mysum, e, a.mycnt), M::mp(a.sum, e, a.cnt);
    if (a.ex.laz_f) M::cp(a.ex.laz, e);
    else a.ex.laz = e, a.ex.laz_f = true;
   }
  }
  [[gnu::always_inline]] void push(int c) {
   if constexpr (AC)
    if (t[c].ex.laz_f) apply_to(t[c].ch[0], t[c].ex.laz), apply_to(t[c].ch[1], t[c].ex.laz), t[c].ex.laz_f = false;
  }
  void push_path(int c) {
   int st[64], k = 0;
   for (int y = c; y && k < 64; y = t[y].par) st[k++] = y;
   while (k) push(st[--k]);
  }
  [[gnu::always_inline]] void rotate(int c) {
   Node& a = t[c];
   const int p = a.par, g = t[p].par, d = t[p].ch[1] == c, b = a.ch[d ^ 1];
   t[p].ch[d] = b;
   if (b) t[b].par = p;
   a.ch[d ^ 1] = p, t[p].par = c, a.par = g;
   if (g) t[g].ch[t[g].ch[1] == p] = c;
   a.tot = t[p].tot, a.cnt = t[p].cnt, a.sum = t[p].sum;
   pull(p);
  }
  void splay_to(int c, int goal) {
   for (int p; (p = t[c].par) != goal;) {
    const int g = t[p].par;
    if (g == goal) {
     push(p), push(c), rotate(c);
     break;
    }
    push(g), push(p), push(c);
    if ((t[g].ch[1] == p) == (t[p].ch[1] == c)) rotate(p), rotate(c);
    else rotate(c), rotate(c);
   }
   push(c);
  }
  void splay(int c) { splay_to(c, 0); }
  int root_of(int c) const {
   while (t[c].par) c = t[c].par;
   return c;
  }
  int rank(int id) {
   const int c = cid[id];
   splay(c);
   return t[t[c].ch[0]].tot + slot[id];
  }
  void reset_chunk(int c) {
   Node& a = t[c];
   a.ch[0] = a.ch[1] = a.par = 0;
   a.len = a.tot = a.cnt = a.mycnt = 0;
   a.mysum = M::ti(), a.sum = M::ti();
   if constexpr (AC) a.ex.laz_f = false;
  }
  int alloc_chunk() {
   if (!free_chunks.empty()) {
    const int c = free_chunks.back();
    free_chunks.pop_back();
    reset_chunk(c);
    return c;
   }
   const int c = int(t.size());
   t.emplace_back();
   reset_chunk(c);
   return c;
  }
  void refresh(int c) {
   Node& a = t[c];
   a.mycnt = 0, a.mysum = M::ti();
   for (int i = 0; i < a.len; ++i)
    if (const int id = a.ids[i]; id <= n) ++a.mycnt, a.mysum = M::op(a.mysum, val[id]);
   pull(c);
  }
  void set_slot(int c, int i, int id) { t[c].ids[i] = id, cid[id] = c, slot[id] = i; }
  // チャンク c を位置 s で 2 つに分ける。前半は c に残し、後半を新しいチャンクで返す。
  int split_chunk(int c, int s) {
   const int d = alloc_chunk();
   Node& a = t[c];
   Node& b = t[d];
   b.len = a.len - s;
   for (int i = 0; i < b.len; ++i) b.ids[i] = a.ids[s + i], cid[b.ids[i]] = d, slot[b.ids[i]] = i;
   a.len = s;
   refresh(c), refresh(d);
   return d;
  }
  // 列を最初の k 要素と残りに分ける。
  void split_pos(int c, int k, int& a, int& b) {
   if (!c) return a = 0, b = 0, void();
   push(c);
   const int ls = t[t[c].ch[0]].tot, clen = t[c].len;
   if (k <= ls) {
    split_pos(t[c].ch[0], k, a, t[c].ch[0]);
    if (t[c].ch[0]) t[t[c].ch[0]].par = c;
    t[c].par = 0, b = c;
    pull(c);
   } else if (k >= ls + clen) {
    split_pos(t[c].ch[1], k - ls - clen, t[c].ch[1], b);
    if (t[c].ch[1]) t[t[c].ch[1]].par = c;
    t[c].par = 0, a = c;
    pull(c);
   } else {
    const int d = split_chunk(c, k - ls);
    t[d].ch[1] = t[c].ch[1];
    if (t[d].ch[1]) t[t[d].ch[1]].par = d;
    t[c].ch[1] = 0;
    pull(c), pull(d);
    a = c, b = d;
    t[a].par = t[b].par = 0;
   }
  }
  // 2 つの列を継ぐ。右端と左端のチャンクが 1 つに収まるならまとめる。
  int merge(int a, int b) {
   if (!a) return b;
   if (!b) return a;
   assert(root_of(a) != root_of(b));
   int x = a;
   push(x);
   while (t[x].ch[1]) x = t[x].ch[1], push(x);
   splay(x);
   int y = b;
   push(y);
   while (t[y].ch[0]) y = t[y].ch[0], push(y);
   splay(y);
   assert(x != y);
   if (t[x].len + t[y].len <= CAP) {
    Node& nx = t[x];
    const int yn = t[y].len;
    for (int i = 0; i < yn; ++i) set_slot(x, nx.len + i, t[y].ids[i]);
    nx.len += yn;
    const int yr = t[y].ch[1];
    if (yr) t[yr].par = x;
    nx.ch[1] = yr;
    refresh(x);
    t[y].ch[0] = t[y].ch[1] = 0, t[y].len = 0, t[y].tot = t[y].cnt = 0;
    free_chunks.push_back(y);
    return x;
   }
   t[x].ch[1] = y, t[y].par = x;
   pull(x);
   return x;
  }
  // 部分木 c の中の位置 [l, r) の和。
  T range_sum(int c, int l, int r) {
   if (!c || l >= r) return M::ti();
   if (l == 0 && r == t[c].tot) return t[c].sum;
   push(c);
   const Node& a = t[c];
   const int ls = t[a.ch[0]].tot;
   T s = M::ti();
   if (l < ls) s = range_sum(a.ch[0], l, std::min(r, ls));
   if (const int lo = std::max(l, ls) - ls, hi = std::min(r, ls + a.len) - ls; lo < hi)
    for (int i = lo; i < hi; ++i)
     if (const int id = a.ids[i]; id <= n) s = M::op(s, val[id]);
   if (r > ls + a.len) s = M::op(s, range_sum(a.ch[1], std::max(l, ls + a.len) - ls - a.len, r - ls - a.len));
   return s;
  }
  template <bool A = AC>
  void range_apply(int c, int l, int r, const E& e) {
   if constexpr (A) {
    if (!c || l >= r) return;
    if (l == 0 && r == t[c].tot) return apply_to(c, e), void();
    push(c);
    Node& a = t[c];
    const int ls = t[a.ch[0]].tot;
    if (l < ls) range_apply(a.ch[0], l, std::min(r, ls), e);
    if (const int lo = std::max(l, ls) - ls, hi = std::min(r, ls + a.len) - ls; lo < hi) {
     for (int i = lo; i < hi; ++i)
      if (const int id = a.ids[i]; id <= n) M::mp(val[id], e, 1);
     refresh(c);
    }
    if (r > ls + a.len) range_apply(a.ch[1], std::max(l, ls + a.len) - ls - a.len, r - ls - a.len, e);
    pull(c);
   }
  }
  int reroot(int id) {
   const int k = rank(id);  // これで id のチャンクが根になる
   int a, b;
   split_pos(cid[id], k, a, b);
   return merge(b, a);
  }
  // 辺の印を作り、その ID と、その 1 個だけを持つチャンクを返す。
  int alloc_edge_node(int& id) {
   id = int(val.size());
   val.push_back(M::ti()), cid.push_back(0), slot.push_back(0);
   const int c = alloc_chunk();
   set_slot(c, 0, id), t[c].len = 1;
   refresh(c);
   return c;
  }
  std::pair<int, int> edge_nodes(int v, int p) const {
   const Edge* found = es.find(key(v, p));
   assert(found);
   if (found->u == v) return {found->a, found->b};
   return {found->b, found->a};
  }
  int build_range(const std::vector<int>& seq, int l, int r) {
   if (l >= r) return 0;
   const int c = alloc_chunk();
   const int m = (l + r) >> 1;
   int lo = m - CAP / 2, hi = lo + CAP;
   if (lo < l) lo = l, hi = std::min(r, lo + CAP);
   if (hi > r) hi = r, lo = std::max(l, hi - CAP);
   for (int i = lo; i < hi; ++i) set_slot(c, i - lo, seq[i]);
   t[c].len = hi - lo;
   refresh(c);
   const int L = build_range(seq, l, lo), R = build_range(seq, hi, r);
   t[c].ch[0] = L, t[c].ch[1] = R;
   if (L) t[L].par = c;
   if (R) t[R].par = c;
   pull(c);
   return c;
  }

 public:
  explicit RopeEulerTourTree(int n_): t(std::size_t(3 * n_ + 64)), val(std::size_t(3 * n_ + 8), M::ti()), cid(std::size_t(3 * n_ + 8), 0), slot(std::size_t(3 * n_ + 8), 0), n(n_) {
   for (int id = 1; id <= n; ++id) {
    const int c = alloc_chunk();
    set_slot(c, 0, id), t[c].len = 1;
    refresh(c);
   }
  }
  RopeEulerTourTree(int n_, const std::vector<std::array<int, 2>>& edges, const std::vector<T>& a)
      : t(std::size_t(3 * n_ + 64)), val(std::size_t(3 * n_ + 8), M::ti()), cid(std::size_t(3 * n_ + 8), 0), slot(std::size_t(3 * n_ + 8), 0), n(n_) {
   for (int v = 1; v <= n; ++v) val[v] = a[v - 1];
   const int E = int(edges.size()), base = n + 1;
   std::vector<int> head(n, -1), to(2 * E), nxt(2 * E), eid(2 * E), dir(2 * E);
   int ec = 0;
   auto add_arc = [&](int u, int v, int e, int d) { to[ec] = v, nxt[ec] = head[u], eid[ec] = e, dir[ec] = d, head[u] = ec++; };
   for (int e = 0; e < E; ++e) {
    const int u = edges[e][0], v = edges[e][1];
    add_arc(u, v, e, 0), add_arc(v, u, e, 1);
    es.insert(key(u, v), Edge{u, v, base + 2 * e, base + 2 * e + 1});
   }
   val.resize(std::size_t(base + 2 * E + 1), M::ti());
   cid.resize(std::size_t(base + 2 * E + 1), 0);
   slot.resize(std::size_t(base + 2 * E + 1), 0);
   std::vector<char> seen(n, 0);
   std::vector<int> seq, su, sp, se, snode;
   seq.reserve(std::size_t(n + 2 * E));
   for (int root = 0; root < n; ++root) {
    if (seen[root]) continue;
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
   }
   build_range(seq, 0, int(seq.size()));
  }
  int size() const { return n; }
  bool edge_exists(int u, int v) const { return es.contains(key(u, v)); }
  T get(int v) {
   const int id = v + 1;
   push_path(cid[id]);
   return val[id];
  }
  void set(int v, T x) {
   const int id = v + 1;
   const int c = cid[id];
   push_path(c);
   val[id] = x;
   refresh(c);
   for (int y = t[c].par; y; y = t[y].par) pull(y);
  }
  void link(int u, int v) {
   assert(u != v && !edge_exists(u, v));
   int ida, idb;
   const int ca = alloc_edge_node(ida), cb = alloc_edge_node(idb);
   const int ru = reroot(u + 1), rv = reroot(v + 1);
   static_cast<void>(merge(merge(merge(ru, ca), rv), cb));
   es.insert(key(u, v), Edge{u, v, ida, idb});
  }
  void cut(int u, int v) {
   const Edge* found = es.find(key(u, v));
   assert(found);
   const Edge e = *found;
   es.erase(key(u, v));
   int a, b;
   if (e.u == u) a = e.a, b = e.b;
   else a = e.b, b = e.a;
   const int ra = rank(a), rb = rank(b);
   const int root = root_of(cid[a]);
   splay(root);
   int p1, p2, p3, p4, p5, mid;
   if (ra > rb) {  // [.. b ..(v の側).. a ..]
    split_pos(root, rb, p1, p2);
    split_pos(p2, 1, mid, p3);
    split_pos(p3, ra - rb - 1, p4, p5);
    split_pos(p5, 1, mid, p5);
    static_cast<void>(merge(p5, p1));
   } else {  // v の側は折り返し (rb,) + [0,ra)
    split_pos(root, ra, p1, p2);
    split_pos(p2, 1, mid, p3);
    split_pos(p3, rb - ra - 1, p4, p5);
    split_pos(p5, 1, mid, p5);
    static_cast<void>(merge(p5, p1));
   }
  }
  bool connected(int u, int v) { return root_of(cid[u + 1]) == root_of(cid[v + 1]); }
  T prod_tree(int v) { return t[root_of(cid[v + 1])].sum; }
  int tree_size(int v) { return t[root_of(cid[v + 1])].cnt; }
  template <bool A = AC>
  void apply_tree(int v, const E& e) {
   static_assert(A, "\"apply_tree\" is not available");
   if constexpr (A) apply_to(root_of(cid[v + 1]), e);
  }
  T prod_subtree(int v, int p) {
   auto [a, b] = edge_nodes(v, p);
   const int ra = rank(a), rb = rank(b);
   const int root = root_of(cid[a]);
   if (ra > rb) return range_sum(root, rb + 1, ra);
   T s = range_sum(root, rb + 1, t[root].tot);
   return M::op(s, range_sum(root, 0, ra));
  }
  template <bool A = AC>
  void apply_subtree(int v, int p, const E& e) {
   static_assert(A, "\"apply_subtree\" is not available");
   if constexpr (A) {
    auto [a, b] = edge_nodes(v, p);
    const int ra = rank(a), rb = rank(b);
    const int root = root_of(cid[a]);
    if (ra > rb) range_apply(root, rb + 1, ra, e);
    else range_apply(root, rb + 1, t[root].tot, e), range_apply(root, 0, ra, e);
   }
  }
};
