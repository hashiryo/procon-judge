#pragma once
#include "common.hpp"

// Fenwick tree (Binary Indexed Tree)。更新も取得も O(log N)。
struct Solver {
  int n;
  vector<i64> bit;

  explicit Solver(int size) : n(size), bit(size, 0) {}

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
