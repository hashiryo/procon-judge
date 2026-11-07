#pragma once
#include "pj.hpp"
#include <limits>

// ACL の lazy_segtree と同じ非再帰の遅延セグ木を、最小値と加算に特化して書いたもの。
// 区間の両端の道を根から配ってから作用と取得をする。大きさは 2 の冪に揃え、余りは +inf。
struct Solver {
  static constexpr i64 INF = numeric_limits<i64>::max() / 4;
  int n, lg, sz;
  vector<i64> d, lz;

  explicit Solver(const vector<i64> &a) : n((int)a.size()) {
    lg = 0;
    while ((1 << lg) < n) ++lg;
    sz = 1 << lg;
    d.assign(2 * sz, INF);
    lz.assign(sz, 0);
    for (int i = 0; i < n; ++i) d[sz + i] = a[i];
    for (int k = sz - 1; k >= 1; --k) d[k] = min(d[2 * k], d[2 * k + 1]);
  }
  void apply_node(int k, i64 x) {
    d[k] += x;
    if (k < sz) lz[k] += x;
  }
  void push(int k) {
    if (lz[k] == 0) return;
    apply_node(2 * k, lz[k]);
    apply_node(2 * k + 1, lz[k]);
    lz[k] = 0;
  }
  void update(int k) { d[k] = min(d[2 * k], d[2 * k + 1]); }

  void add(int l, int r, i64 x) {
    l += sz, r += sz;
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i);
      if (((r >> i) << i) != r) push((r - 1) >> i);
    }
    for (int a = l, b = r; a < b; a >>= 1, b >>= 1) {
      if (a & 1) apply_node(a++, x);
      if (b & 1) apply_node(--b, x);
    }
    for (int i = 1; i <= lg; ++i) {
      if (((l >> i) << i) != l) update(l >> i);
      if (((r >> i) << i) != r) update((r - 1) >> i);
    }
  }
  i64 minimum(int l, int r) {
    l += sz, r += sz;
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i);
      if (((r >> i) << i) != r) push((r - 1) >> i);
    }
    i64 res = INF;
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) res = min(res, d[l++]);
      if (r & 1) res = min(res, d[--r]);
    }
    return res;
  }
};
