#pragma once
// オフラインの動的連結性。全クエリを先に読み、辺の削除時刻を重みにした最大全域森を 1 本持つ。
// Library Checker の dynamic_graph_vertex_add_component_sum の 1 位 (nandhagk の 402761) と、
// dynamic_tree_vertex_add_subtree_sum の 1 位 (Rohan_Kapri の 405118) の核を、可換群を受ける形にした。
//   Operation: kind 0 = link(a,b)、1 = cut(a,b)、2 = 頂点 a に b を足す、3 = a の成分の和。
//   G は可換群 (using T、static T ti()、static T op(T,T)、static T inv(T)、using commute)。
// 森は親ポインタと部分木の大きさ・部分木和だけを持ち、splay をしない。2/3 の大きさ平衡 (anti-monopoly) を
// 保つ。辺の期限は削除時刻で、期限の小さい辺ほど上に来る。削除は owner[時刻] で切る節を引き、代替辺を探さない。
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>
#include "_shared/dyntree/common.hpp"

namespace offline_dyn {
struct Operation {
 unsigned kind, a, b;
};
// 各 link の削除時刻 (イベント番号)。削除されない辺はイベント数。
// 辺の寿命は、2 の冪の大きさの開番地法の表で引く。std::unordered_map より軽い。
class Timeline {
 std::vector<Operation> ops_;
 std::vector<unsigned> death_;
 static constexpr unsigned long long TOMB = ~0ull;

 public:
 explicit Timeline(const std::vector<Operation>& ops): ops_(ops), death_(ops.size(), unsigned(ops.size())) {
  std::size_t links = 0;
  for (const Operation& o : ops) links += (o.kind == 0);
  std::size_t cap = 4, shift = 62;
  while (cap < links * 2) cap <<= 1, --shift;
  std::vector<unsigned long long> keys(cap, 0);
  std::vector<unsigned> vals(cap, 0);
  auto hash = [shift](unsigned long long k) { return std::size_t((k * 0x9E3779B97F4A7C15ull) >> shift); };
  for (unsigned i = 0; i < ops.size(); ++i) {
   const Operation& o = ops[i];
   if (o.kind > 1) continue;
   unsigned u = o.a, v = o.b;
   if (u > v) std::swap(u, v);
   const unsigned long long key = (unsigned long long)(u + 1) << 32 | (v + 1);
   std::size_t h = hash(key);
   if (o.kind == 0) {
    std::size_t tomb = cap;
    while (keys[h]) {
     if (keys[h] == TOMB && tomb == cap) tomb = h;
     h = (h + 1) & (cap - 1);
    }
    if (tomb != cap) h = tomb;
    keys[h] = key, vals[h] = i;
   } else {
    while (keys[h] != key) h = (h + 1) & (cap - 1);
    death_[vals[h]] = i;
    keys[h] = TOMB;
   }
  }
 }
 unsigned death(unsigned t) const { return death_[t]; }
};

template <class G>
class Forest {
 using T = typename G::T;
 struct Node {
  unsigned parent = 0, deadline = 0, size = 1;
  T sum;
 };
 std::vector<Node> nd;
 std::vector<unsigned> owner;
 unsigned horizon;
 void unindex(unsigned x) {
  const unsigned d = nd[x].deadline;
  if (d && d < horizon) {
   assert(owner[d] == x);
   owner[d] = 0;
  }
 }
 void index(unsigned x) {
  const unsigned d = nd[x].deadline;
  if (d && d < horizon) {
   assert(!owner[d]);
   owner[d] = x;
  }
 }
 [[gnu::always_inline]] void rotate(unsigned x) {
  Node& a = nd[x];
  const unsigned y = a.parent;
  Node& b = nd[y];
  b.size -= a.size;
  b.sum = G::op(b.sum, G::inv(a.sum));
  if (a.deadline <= b.deadline) a.parent = b.parent;
  else {
   a.size += b.size;
   a.sum = G::op(a.sum, b.sum);
   unindex(x), unindex(y);
   std::swap(a.deadline, b.deadline);
   a.parent = b.parent;
   b.parent = x;
   index(x), index(y);
  }
 }
 [[gnu::always_inline]] void maintain(unsigned x) {
  while (nd[x].parent) {
   const unsigned y = nd[x].parent;
   if ((unsigned long long)nd[x].size * 3 <= (unsigned long long)nd[y].size * 2) x = y;
   else rotate(x);
  }
 }
 [[gnu::always_inline]] void reroot(unsigned x, unsigned watch) {
  while (nd[x].parent && nd[x].parent != watch) rotate(x);
 }

 public:
 explicit Forest(const std::vector<T>& values, unsigned horizon_): nd(values.size() + 1), owner(horizon_), horizon(horizon_) {
  nd[0].size = 0;
  nd[0].sum = G::ti();
  for (unsigned i = 0; i < values.size(); ++i) nd[i + 1].sum = values[i];
 }
 void insert(unsigned u, unsigned v, unsigned deadline) {
  ++u, ++v;
  assert(u != v && deadline && deadline <= horizon);
  maintain(u), maintain(v);
  reroot(v, 0);
  reroot(u, v);
  if (nd[u].parent == v) {
   if (deadline > nd[u].deadline) {
    unindex(u);
    nd[u].deadline = deadline;
    index(u);
   }
  } else {
   assert(!nd[u].parent);
   if (nd[u].size > nd[v].size) std::swap(u, v);
   nd[u].parent = v;
   nd[u].deadline = deadline;
   index(u);
   nd[v].size += nd[u].size;
   nd[v].sum = G::op(nd[v].sum, nd[u].sum);
  }
 }
 void expire(unsigned deadline) {
  assert(deadline < horizon);
  const unsigned x = owner[deadline];
  if (!x) return;
  const unsigned count = nd[x].size;
  const T sum = nd[x].sum;
  for (unsigned p = nd[x].parent; p; p = nd[p].parent) {
   nd[p].size -= count;
   nd[p].sum = G::op(nd[p].sum, G::inv(sum));
  }
  nd[x].parent = 0;
  nd[x].deadline = 0;
  owner[deadline] = 0;
 }
 void add(unsigned v, const T& delta) {
  ++v;
  maintain(v);
  for (; v; v = nd[v].parent) nd[v].sum = G::op(nd[v].sum, delta);
 }
 T fold(unsigned v) {
  ++v;
  maintain(v);
  while (nd[v].parent) v = nd[v].parent;
  return nd[v].sum;
 }
};

// 操作を先頭から再生する。答えは kind 3 の順。
template <class G>
std::vector<typename G::T> solve(const std::vector<typename G::T>& initial, const std::vector<Operation>& ops) {
 Timeline timeline(ops);
 Forest<G> forest(initial, unsigned(ops.size()));
 std::vector<typename G::T> ans;
 for (unsigned t = 0; t < ops.size(); ++t) {
  const Operation& o = ops[t];
  if (o.kind == 0) forest.insert(o.a, o.b, timeline.death(t));
  else if (o.kind == 1) forest.expire(t);
  else if (o.kind == 2) forest.add(o.a, typename G::T(o.b));
  else ans.push_back(forest.fold(o.a));
 }
 return ans;
}
}  // namespace offline_dyn
