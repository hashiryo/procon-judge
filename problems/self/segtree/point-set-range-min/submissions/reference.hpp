#pragma once
#include "pj.hpp"
#include <limits>

// 参照実装。葉を [n, 2n) に置く非再帰のセグメント木 (Library に依存しない)。
struct Solver {
  int n;
  vector<i64> t;
  explicit Solver(const vector<i64> &a) : n((int)a.size()), t(2 * a.size()) {
    for (int i = 0; i < n; ++i) t[n + i] = a[i];
    for (int k = n - 1; k >= 1; --k) t[k] = min(t[2 * k], t[2 * k + 1]);
  }
  void set(int p, i64 x) {
    for (t[p += n] = x; p > 1; p >>= 1) t[p >> 1] = min(t[p], t[p ^ 1]);
  }
  i64 minimum(int l, int r) {
    i64 res = numeric_limits<i64>::max();
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
      if (l & 1) res = min(res, t[l++]);
      if (r & 1) res = min(res, t[--r]);
    }
    return res;
  }
};
