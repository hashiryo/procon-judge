#pragma once
#include "pj.hpp"
#include "_shared/kinetic/beats.hpp"

// 同じ集合を全区間に作用する kinetic segment tree で持つ形。時刻の前進を、全区間への a の加算にする。
struct Solver {
  KineticBeats<true> s;
  int n;
  i64 now = 0;
  explicit Solver(const vector<i64> &a, const vector<i64> &b) : n(int(a.size())) { s.init(a, b); }
  void set(int i, i64 a, i64 b) { s.set(i, a, a * now + b); }
  i64 query(i64 t) {
    if (t > now) s.add(0, n, t - now, 0), now = t;
    return s.all();
  }
};
