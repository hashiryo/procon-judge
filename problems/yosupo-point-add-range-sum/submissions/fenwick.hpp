#pragma once
#include "common.hpp"

// Fenwick tree (Binary Indexed Tree)。更新も取得も O(log N)。
struct Solver {
  int n;
  vector<i64> bit;

  explicit Solver(const vector<i64> &init) : n((int)init.size()), bit(init) {
    // in-place build。i の親 i + (i & -i) に足していく。
    for (int i = 1; i <= n; ++i) {
      int j = i + (i & -i);
      if (j <= n) bit[j - 1] += bit[i - 1];
    }
  }

  void add(int p, i64 x) {
    for (int i = p + 1; i <= n; i += i & -i) bit[i - 1] += x;
  }

  i64 prefix(int r) {
    i64 s = 0;
    for (int i = r; i > 0; i -= i & -i) s += bit[i - 1];
    return s;
  }

  i64 sum(int l, int r) { return prefix(r) - prefix(l); }
};
