#pragma once
#include "common.hpp"

// 配列をそのまま持って、区間和は毎回足し直す。O(1) 更新 / O(N) 取得。
// N, Q ともに 10 万までなので、この問題では間に合ってしまう。
struct Solver {
  vector<i64> a;

  explicit Solver(int n) : a(n, 0) {}

  void add(int p, i64 x) { a[p] += x; }

  i64 sum(int l, int r) {
    i64 s = 0;
    for (int i = l; i < r; ++i) s += a[i];
    return s;
  }
};
