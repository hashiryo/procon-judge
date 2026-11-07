#pragma once
#include "pj.hpp"
#include <limits>

// 参照実装。ACL と同じ非再帰の遅延セグ木で、代入の作用を別の配列の印で持つ (Library に依存しない)。
struct Solver {
  int n, lg, sz;
  vector<i64> d, lz;
  vector<char> fl;
  static i64 op(i64 a, i64 b) { return min(a, b); }
  explicit Solver(const vector<i64> &a) : n((int)a.size()) {
    lg = 0;
    while ((1 << lg) < n) ++lg;
    sz = 1 << lg;
    d.assign(2 * sz, numeric_limits<i64>::max()), lz.assign(sz, 0), fl.assign(sz, 0);
    for (int i = 0; i < n; ++i) d[sz + i] = a[i];
    for (int k = sz - 1; k >= 1; --k) d[k] = op(d[2 * k], d[2 * k + 1]);
  }
  void all_apply(int k, i64 x, i64 len) {
    (void)len;
    d[k] = x;
    if (k < sz) lz[k] = x, fl[k] = 1;
  }
  void push(int k, i64 len) {
    if (!fl[k]) return;
    all_apply(2 * k, lz[k], len / 2), all_apply(2 * k + 1, lz[k], len / 2), fl[k] = 0;
  }
  void pushes(int l, int r) {
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i, i64(1) << i);
      if (((r >> i) << i) != r) push((r - 1) >> i, i64(1) << i);
    }
  }
  void assign(int l, int r, i64 x) {
    l += sz, r += sz;
    pushes(l, r);
    for (int a = l, b = r, len = 1; a < b; a >>= 1, b >>= 1, len <<= 1) {
      if (a & 1) all_apply(a++, x, len);
      if (b & 1) all_apply(--b, x, len);
    }
    for (int i = 1; i <= lg; ++i) {
      if (((l >> i) << i) != l) d[l >> i] = op(d[2 * (l >> i)], d[2 * (l >> i) + 1]);
      if (((r >> i) << i) != r) d[(r - 1) >> i] = op(d[2 * ((r - 1) >> i)], d[2 * ((r - 1) >> i) + 1]);
    }
  }
  i64 minimum(int l, int r) {
    l += sz, r += sz;
    pushes(l, r);
    i64 sl = numeric_limits<i64>::max(), sr = numeric_limits<i64>::max();
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) sl = op(sl, d[l++]);
      if (r & 1) sr = op(d[--r], sr);
    }
    return op(sl, sr);
  }
};
