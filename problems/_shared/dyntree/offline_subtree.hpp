#pragma once
// オフラインの部分木加算と部分木和。Library Checker の dynamic_tree_subtree_add_subtree_sum の
// 1 位 (anon123 の 401546) の核 (FoldableAntiMonopolyTree) を、そのまま写したもの。
// 頂点 1 個 1 節のクラスタ木 (top tree の一種) で、親・部分木の大きさ・親辺の重み・部分木和・遅延を持つ。
// 2/3 の大きさ平衡を保ち、splay を使わない。辺の重みは削除時刻の符号反転で、閉路では重みの大きい辺を外す。
// 部分木は辺 (v,p) を切った成分で、イベント列の再生で切る・足す・聞く・戻す。
#include <cassert>
#include <climits>
#include <cstdint>
#include <utility>
#include <vector>

namespace offline_sub {
class FoldableAmTree {
 int n;
 std::vector<int> parent_, size_, weight_;
 std::vector<long long> sum_, lazy_;

 public:
 // a は頂点ごとの初期値。
 explicit FoldableAmTree(const std::vector<long long>& a)
     : n(int(a.size())), parent_(n, -1), size_(n, 1), weight_(n, INT_MAX), sum_(a), lazy_(n, 0) {}
 int size() const { return n; }
 long long value(int v) const { return sum_[v] + potential(v) - potential(v); }
 long long potential(int v) const {
  long long x = 0;
  for (; ~v; v = parent_[v]) x += lazy_[v];
  return x;
 }
 int root(int v) const {
  while (~parent_[v]) v = parent_[v];
  return v;
 }
 [[gnu::always_inline]] void promote(int v) {
  int p = parent_[v];
  int len = size_[v];
  long long x = lazy_[p];
  size_[p] -= len;
  sum_[p] -= sum_[v] + x * len;
  parent_[v] = parent_[p];
  lazy_[v] += x;
  sum_[v] += x * len;
  if (weight_[v] < weight_[p]) {
   x = lazy_[v];
   lazy_[p] -= x;
   sum_[p] -= x * size_[p];
   size_[v] += size_[p];
   sum_[v] += sum_[p] + x * size_[p];
   std::swap(weight_[v], weight_[p]);
   parent_[p] = v;
  }
 }
 [[gnu::always_inline]] void upward_maintain(int v) {
  while (~parent_[v]) {
   int p = parent_[v];
   if (3LL * size_[v] <= 2LL * size_[p]) {
    v = p;
    continue;
   }
   promote(v);
  }
 }
 // 経路 u-v の最大の重みと、その節点。非連結なら {INT_MAX, -1}。
 std::pair<int, int> path_max(int u, int v) {
  upward_maintain(u), upward_maintain(v);
  int max_w = INT_MIN, t = -1;
  while (u != v) {
   if (size_[u] > size_[v]) std::swap(u, v);
   if (!~parent_[u]) return {INT_MAX, -1};
   if (weight_[u] > max_w) max_w = weight_[u], t = u;
   u = parent_[u];
  }
  return {max_w, t};
 }
 // v の親辺を切る。
 void cut(int v) {
  int len = size_[v];
  long long x = 0;
  for (int p = parent_[v]; ~p; p = parent_[p]) x += lazy_[p];
  int p = parent_[v];
  if (~p) {
   int dsize = -len;
   long long dsum = -(sum_[v] + lazy_[p] * len);
   for (;;) {
    size_[p] += dsize;
    sum_[p] += dsum;
    int q = parent_[p];
    if (!~q) break;
    dsum += lazy_[q] * dsize;
    p = q;
   }
  }
  lazy_[v] += x;
  sum_[v] += x * len;
  parent_[v] = -1;
  weight_[v] = INT_MAX;
 }
 // 辺 (u,v) を重み w で入れる。閉路なら経路の最大重みの辺を外す。連結したら真。
 bool add(int u, int v, int w) {
  if (u == v) return false;
  auto [max_w, t] = path_max(u, v);
  bool merged = max_w == INT_MAX;
  if (!merged) {
   if (w >= max_w) return false;
   cut(t);
  }
  long long pu = potential(u), pv = potential(v);
  int du = 0, dv = 0;
  long long su = 0, sv = 0;
  for (;;) {
   if (w >= weight_[u]) {
    int p = parent_[u];
    su += lazy_[p] * du;
    size_[p] += du;
    sum_[p] += su;
    pu -= lazy_[u];
    u = p;
   } else if (w >= weight_[v]) {
    int p = parent_[v];
    sv += lazy_[p] * dv;
    size_[p] += dv;
    sum_[p] += sv;
    pv -= lazy_[v];
    v = p;
   } else {
    if (size_[u] > size_[v]) {
     std::swap(u, v), std::swap(du, dv), std::swap(su, sv), std::swap(pu, pv);
    }
    int len = size_[u];
    long long value = sum_[u];
    long long old_lazy = lazy_[u];
    int old_parent = parent_[u];
    int old_weight = weight_[u];
    if (~old_parent) {
     long long next_sum = su + lazy_[old_parent] * du;
     du -= len;
     su = next_sum - value - lazy_[old_parent] * len;
     size_[old_parent] += du;
     sum_[old_parent] += su;
    }
    long long delta = pu - old_lazy - pv;
    lazy_[u] += delta;
    sum_[u] += delta * len;
    long long value_v = sum_[u] + lazy_[v] * len;
    dv += len;
    sv += value_v;
    size_[v] += len;
    sum_[v] += value_v;
    parent_[u] = v;
    weight_[u] = w;
    w = old_weight;
    if (!~old_parent) {
     for (; ~parent_[v]; v = parent_[v]) {
      int p = parent_[v];
      sv += lazy_[p] * dv;
      size_[p] += dv;
      sum_[p] += sv;
     }
     return merged;
    }
    pu -= old_lazy;
    u = old_parent;
   }
  }
 }
 // 辺 (u,v) の重み w の辺を外す。経路の最大重みの辺がそれより大きいときだけ切る。
 bool erase(int u, int v, int w) {
  auto [max_w, t] = path_max(u, v);
  if (!~t || w >= max_w) return false;
  cut(t);
  return true;
 }
 void add_vertex(int v, long long x) {
  upward_maintain(v);
  for (; ~v; v = parent_[v]) sum_[v] += x;
 }
 // v の成分全体に x を足す。
 void add_all(int v, long long x) {
  upward_maintain(v);
  int r = root(v);
  lazy_[r] += x;
  sum_[r] += x * size_[r];
 }
 // v の成分の和。
 long long fold(int v) {
  upward_maintain(v);
  return sum_[root(v)];
 }
};

// イベント列。kind 0 = link、1 = cut、2 = 成分全体への加算、3 = 成分の和。
struct Event {
 int kind, u, v;
 long long w;
};
// 生きている辺のイベント番号。2 の冪の大きさの開番地法の表で、埋まりが半分を超えたら広げる。
// 消した跡は墓標にして、広げ直すときに落とす。
class EdgeIndex {
 struct Slot {
  unsigned long long k;
  int v;
 };
 std::vector<Slot> t;
 std::size_t mask, shift, live, used;
 static constexpr unsigned long long TOMB = ~0ull;
 std::size_t home(unsigned long long k) const { return std::size_t((k * 0x9E3779B97F4A7C15ull) >> shift); }
 void put(unsigned long long k, int v) {
  std::size_t h = home(k);
  while (t[h].k && t[h].k != TOMB) h = (h + 1) & mask;
  if (!t[h].k) ++used;
  t[h].k = k, t[h].v = v, ++live;
 }
 void rehash(std::size_t cap, std::size_t new_shift) {
  std::vector<Slot> old;
  old.swap(t);
  t.assign(cap, Slot{0, 0});
  mask = cap - 1, shift = new_shift, live = 0, used = 0;
  for (const Slot& s : old)
   if (s.k && s.k != TOMB) put(s.k, s.v);
 }

 public:
 EdgeIndex(): t(16, Slot{0, 0}), mask(15), shift(60), live(0), used(0) {}
 void insert(unsigned long long k, int v) {
  if ((used + 1) * 2 > mask + 1) {
   if ((live + 1) * 2 <= mask + 1) rehash(mask + 1, shift);
   else rehash((mask + 1) * 2, shift - 1);
  }
  put(k, v);
 }
 // 無ければ -1。
 int find(unsigned long long k) const {
  std::size_t h = home(k);
  while (t[h].k) {
   if (t[h].k == k) return t[h].v;
   h = (h + 1) & mask;
  }
  return -1;
 }
 void erase(unsigned long long k) {
  std::size_t h = home(k);
  while (t[h].k != k) h = (h + 1) & mask;
  t[h].k = TOMB, --live;
 }
};
class OfflineSubtree {
 EdgeIndex active;
 std::vector<Event> ev;
 static unsigned long long key(int u, int v) {
  if (u > v) std::swap(u, v);
  return (unsigned long long)(unsigned)(u + 1) << 32 | (unsigned)(v + 1);
 }

 public:
 explicit OfflineSubtree(int) { ev.reserve(1024); }
 void link(int u, int v) {
  active.insert(key(u, v), int(ev.size()));
  ev.push_back({0, u, v, INT_MIN});
 }
 void cut(int u, int v) {
  const int idx = active.find(key(u, v));
  assert(idx >= 0);
  ev[idx].w = -(long long)ev.size();
  active.erase(key(u, v));
  ev.push_back({1, u, v, 1});
 }
 void add_all(int v, long long x) { ev.push_back({2, v, 0, x}); }
 void fold(int v) { ev.push_back({3, v, 0, 0}); }
 std::vector<long long> solve(const std::vector<long long>& a) {
  FoldableAmTree amt(a);
  std::vector<long long> ans;
  for (int i = 0; i < int(ev.size()); ++i) {
   const Event& e = ev[i];
   if (e.kind == 0) amt.add(e.u, e.v, int(e.w));
   else if (e.kind == 1) amt.erase(e.u, e.v, -(i + 1));
   else if (e.kind == 2) amt.add_all(e.u, e.w);
   else ans.push_back(amt.fold(e.u));
  }
  return ans;
 }
};
}  // namespace offline_sub
