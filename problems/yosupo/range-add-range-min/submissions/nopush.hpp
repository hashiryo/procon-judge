#pragma once
#include "pj.hpp"
#include <limits>

// 加算は最小値と可換なので、作用を子へ配らない非再帰のセグメント木。節 k の値は
// t[k] = min(t[2k], t[2k+1]) + d[k] で、d[k] はその部分木全体に足した量。区間の加算は覆う節の
// t と d に足し、両端の道を根まで取り直す。区間の最小は、下から集めた左右の最小に、集めた要素を
// 含む祖先 (左は l、右は r - 1 の祖先) の d を段ごとに足していく。大きさは 2 の冪で、余りは +inf。
struct Solver {
  static constexpr i64 INF = numeric_limits<i64>::max() / 4;
  int n, lg, sz;
  vector<i64> t, d;

  explicit Solver(const vector<i64> &a) : n((int)a.size()) {
    lg = 0;
    while ((1 << lg) < n) ++lg;
    sz = 1 << lg;
    t.assign(2 * sz, INF);
    d.assign(sz, 0);
    for (int i = 0; i < n; ++i) t[sz + i] = a[i];
    for (int k = sz - 1; k >= 1; --k) t[k] = min(t[2 * k], t[2 * k + 1]);
  }
  void rebuild(int k) {
    for (k >>= 1; k >= 1; k >>= 1) t[k] = min(t[2 * k], t[2 * k + 1]) + d[k];
  }

  void add(int l, int r, i64 x) {
    l += sz, r += sz;
    const int l0 = l, r0 = r - 1;
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) {
        t[l] += x;
        if (l < sz) d[l] += x;
        ++l;
      }
      if (r & 1) {
        --r;
        t[r] += x;
        if (r < sz) d[r] += x;
      }
    }
    rebuild(l0), rebuild(r0);
  }
  i64 minimum(int l, int r) {
    l += sz, r += sz;
    const int l0 = l, r0 = r - 1;
    i64 rl = INF, rr = INF;
    for (int i = 0;; ++i, l >>= 1, r >>= 1) {
      if (l < r) {
        if (l & 1) rl = min(rl, t[l++]);
        if (r & 1) rr = min(rr, t[--r]);
      }
      if (i == lg) break;
      // 段 i で左に集めた要素は l0 の段 i + 1 の祖先の下にある (右は r0 の祖先)。
      rl += d[l0 >> (i + 1)];
      rr += d[r0 >> (i + 1)];
    }
    return min(rl, rr);
  }
};
