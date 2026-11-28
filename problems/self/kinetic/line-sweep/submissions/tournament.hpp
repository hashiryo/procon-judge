#pragma once
#include "pj.hpp"
#include "_shared/kinetic/tournament.hpp"

// イベント駆動の kinetic tournament tree。入れ替わりの時刻を優先度付きキューに積む。
struct Solver {
  KineticTournament tp;
  explicit Solver(const vector<i64> &a, const vector<i64> &b) { tp.init(a, b, 0); }
  void set(int i, i64 a, i64 b) { tp.set(i, a, b); }
  i64 query(i64 t) { return tp.query(t); }
};
