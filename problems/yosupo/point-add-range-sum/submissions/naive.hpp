#pragma once
#include "pj.hpp"

// 配列をそのまま持って、区間和は毎回足し直す。O(1) 更新 / O(N) 取得。
// 比較の下限を置くための実装なので、大きいケースでは TLE する。
struct Solver {
  vector<i64> a;

  explicit Solver(const vector<i64> &init) : a(init) {}

  void add(int p, i64 x) { a[p] += x; }

  i64 sum(int l, int r) {
    i64 s = 0;
    for (int i = l; i < r; ++i) s += a[i];
    return s;
  }
};
