#pragma once
// NeoLibrary の assignment (neo/graph/assignment.hpp) で書いたもの。中身は hungarian_avx2_cr と同じで、双対を返す分と、行ごとの
// 列を返す形が違う。
#include "neo/graph/assignment.hpp"
struct Solver {
 int n;
 const vector<i64>& a;
 Assignment<i64> r;
 Solver(int n, const vector<i64>& a): n(n), a(a) {}
 void run() { r= ::assignment(n, n, a); }
 i64 cost() const { return r.cost; }
 const vector<int>& assignment() const { return r.col; }
};
