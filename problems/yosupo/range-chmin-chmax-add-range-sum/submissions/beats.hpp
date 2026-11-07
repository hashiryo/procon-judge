#pragma once
#include "pj.hpp"
#include <limits>

// Segment Tree Beats を、この問題の 4 つの操作に特化して書いたもの。節は最大と 2 番目の最大、最小と
// 2 番目の最小、和、まだ配っていない加算の量、最大と最小の個数を 64 byte に並べる。根から再帰で
// 下り、chmin は最大だけを、chmax は最小だけを動かせば済む節で止める。配るときは加算を先に当て、
// 子の最大が親の最大を超えていれば子にも chmin を当てる (chmax も同じ)。大きさは 2 の冪で、余りの
// 葉は最大 -inf、最小 +inf、個数 0 にして、範囲の外として扱う (加算の作用はそこへ届かない)。
struct Solver {
  static constexpr i64 INF = numeric_limits<i64>::max() / 4;
  struct alignas(64) Node {
    i64 mx, mx2, mn, mn2, sum, lz;
    int mxc, mnc;
  };
  int n, sz;
  vector<Node> t;

  explicit Solver(const vector<i64> &a) : n((int)a.size()) {
    sz = 1;
    while (sz < n) sz <<= 1;
    t.resize(2 * sz);
    for (int i = 0; i < sz; ++i) {
      Node &X = t[sz + i];
      if (i < n) X = {a[i], -INF, a[i], INF, a[i], 0, 1, 1};
      else X = {-INF, -INF, INF, INF, 0, 0, 0, 0};
    }
    for (int k = sz - 1; k >= 1; --k) t[k].lz = 0, pull(k);
  }

  void pull(int k) {
    const Node &L = t[2 * k], &R = t[2 * k + 1];
    Node &X = t[k];
    X.sum = L.sum + R.sum;
    if (L.mx > R.mx) X.mx = L.mx, X.mxc = L.mxc, X.mx2 = max(L.mx2, R.mx);
    else if (L.mx < R.mx) X.mx = R.mx, X.mxc = R.mxc, X.mx2 = max(L.mx, R.mx2);
    else X.mx = L.mx, X.mxc = L.mxc + R.mxc, X.mx2 = max(L.mx2, R.mx2);
    if (L.mn < R.mn) X.mn = L.mn, X.mnc = L.mnc, X.mn2 = min(L.mn2, R.mn);
    else if (L.mn > R.mn) X.mn = R.mn, X.mnc = R.mnc, X.mn2 = min(L.mn, R.mn2);
    else X.mn = L.mn, X.mnc = L.mnc + R.mnc, X.mn2 = min(L.mn2, R.mn2);
  }
  void apply_add(int k, int len, i64 x) {
    Node &X = t[k];
    X.sum += x * len, X.mx += x, X.mn += x, X.lz += x;
    if (X.mx2 != -INF) X.mx2 += x;
    if (X.mn2 != INF) X.mn2 += x;
  }
  // X.mx2 < x < X.mx のときだけ呼ぶ。
  void apply_chmin(int k, i64 x) {
    Node &X = t[k];
    X.sum -= (X.mx - x) * X.mxc;
    if (X.mn == X.mx) X.mn = x;
    else if (X.mn2 == X.mx) X.mn2 = x;
    X.mx = x;
  }
  // X.mn < x < X.mn2 のときだけ呼ぶ。
  void apply_chmax(int k, i64 x) {
    Node &X = t[k];
    X.sum += (x - X.mn) * X.mnc;
    if (X.mx == X.mn) X.mx = x;
    else if (X.mx2 == X.mn) X.mx2 = x;
    X.mn = x;
  }
  void push(int k, int len) {
    Node &X = t[k];
    if (X.lz) {
      apply_add(2 * k, len >> 1, X.lz), apply_add(2 * k + 1, len >> 1, X.lz);
      X.lz = 0;
    }
    for (int c = 2 * k; c <= 2 * k + 1; ++c) {
      if (t[c].mx > X.mx) apply_chmin(c, X.mx);
      if (t[c].mn < X.mn) apply_chmax(c, X.mn);
    }
  }

  void chmin_rec(int k, int nl, int nr, int l, int r, i64 x) {
    if (r <= nl || nr <= l || t[k].mx <= x) return;
    if (l <= nl && nr <= r && t[k].mx2 < x) return apply_chmin(k, x);
    push(k, nr - nl);
    const int m = (nl + nr) >> 1;
    chmin_rec(2 * k, nl, m, l, r, x), chmin_rec(2 * k + 1, m, nr, l, r, x);
    pull(k);
  }
  void chmax_rec(int k, int nl, int nr, int l, int r, i64 x) {
    if (r <= nl || nr <= l || t[k].mn >= x) return;
    if (l <= nl && nr <= r && t[k].mn2 > x) return apply_chmax(k, x);
    push(k, nr - nl);
    const int m = (nl + nr) >> 1;
    chmax_rec(2 * k, nl, m, l, r, x), chmax_rec(2 * k + 1, m, nr, l, r, x);
    pull(k);
  }
  void add_rec(int k, int nl, int nr, int l, int r, i64 x) {
    if (r <= nl || nr <= l) return;
    if (l <= nl && nr <= r) return apply_add(k, nr - nl, x);
    push(k, nr - nl);
    const int m = (nl + nr) >> 1;
    add_rec(2 * k, nl, m, l, r, x), add_rec(2 * k + 1, m, nr, l, r, x);
    pull(k);
  }
  i64 sum_rec(int k, int nl, int nr, int l, int r) {
    if (r <= nl || nr <= l) return 0;
    if (l <= nl && nr <= r) return t[k].sum;
    push(k, nr - nl);
    const int m = (nl + nr) >> 1;
    return sum_rec(2 * k, nl, m, l, r) + sum_rec(2 * k + 1, m, nr, l, r);
  }

  void chmin(int l, int r, i64 b) { chmin_rec(1, 0, sz, l, r, b); }
  void chmax(int l, int r, i64 b) { chmax_rec(1, 0, sz, l, r, b); }
  void add(int l, int r, i64 b) { add_rec(1, 0, sz, l, r, b); }
  i64 sum(int l, int r) { return sum_rec(1, 0, sz, l, r); }
};
