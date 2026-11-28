#pragma once
#include "pj.hpp"
#include "_shared/kinetic/sqrt_hull.hpp"

// 参照実装。平方分割と塊ごとの凸包。計算量の保証があり、時刻の進み方によらない。
struct Solver {
  SqrtHull<true> s;
  int n;
  i64 now = 0;
  explicit Solver(const vector<i64> &a, const vector<i64> &b) : n(int(a.size())) { s.init(a, b); }
  void set(int i, i64 a, i64 b) { s.set(i, a, a * now + b); }
  i64 query(i64 t) {
    if (t > now) s.add(0, n, t - now, 0), now = t;
    return s.all();
  }
};
