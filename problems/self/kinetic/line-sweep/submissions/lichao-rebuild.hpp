#pragma once
#include "pj.hpp"

// 直線を置き換えるたびに Li Chao 木を全部の直線から組み直す形。問いが多く置き換えが少ない入力では速い。
// 時刻の単調性は使わない。木は [0, 10^9] を持ち、節は使うときに作る。
struct Solver {
  using i128 = __int128_t;
  struct Node {
    i64 a = 0, b = 0;
    int l = 0, r = 0;
    bool has = false;
  };
  static constexpr i64 LO = 0, HI = 1000000000;
  vector<i64> A, B;
  vector<Node> t;
  bool dirty = true;
  int n;
  explicit Solver(const vector<i64> &a, const vector<i64> &b) : A(a), B(b), n(int(a.size())) {}
  static i128 val(i64 a, i64 b, i64 x) { return (i128)a * x + b; }
  void insert(i64 a, i64 b) {
    if (!t[0].has) {
      t[0] = Node{a, b, 0, 0, true};
      return;
    }
    int k = 0;
    i64 lo = LO, hi = HI;
    while (true) {
      if (!t[k].has) {
        t[k] = Node{a, b, 0, 0, true};
        return;
      }
      i64 m = lo + (hi - lo) / 2;
      bool at_lo = val(a, b, lo) > val(t[k].a, t[k].b, lo);
      bool at_mid = val(a, b, m) > val(t[k].a, t[k].b, m);
      if (at_mid) swap(t[k].a, a), swap(t[k].b, b);
      if (lo == hi) return;
      if (at_lo != at_mid) {
        if (!t[k].l) t.resize(t.size() + 1), t[k].l = int(t.size()) - 1;
        k = t[k].l, hi = m;
      } else {
        if (!t[k].r) t.resize(t.size() + 1), t[k].r = int(t.size()) - 1;
        k = t[k].r, lo = m + 1;
      }
    }
  }
  void rebuild() {
    t.assign(1, Node{});
    for (int i = 0; i < n; ++i) insert(A[i], B[i]);
    dirty = false;
  }
  void set(int i, i64 a, i64 b) { A[i] = a, B[i] = b, dirty = true; }
  i64 query(i64 x) {
    if (dirty) rebuild();
    i128 res = -((i128)1 << 120);
    int k = 0;
    i64 lo = LO, hi = HI;
    while (true) {
      if (!t[k].has) break;
      res = max(res, val(t[k].a, t[k].b, x));
      if (lo == hi) break;
      i64 m = lo + (hi - lo) / 2;
      if (x <= m) {
        if (!t[k].l) break;
        k = t[k].l, hi = m;
      } else {
        if (!t[k].r) break;
        k = t[k].r, lo = m + 1;
      }
    }
    return (i64)res;
  }
};
