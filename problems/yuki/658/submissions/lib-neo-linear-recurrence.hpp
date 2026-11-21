#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/compute/poly/linear_recurrence.hpp"
// NeoLibrary の linear_recurrence (Bostan-Mori) を ZMod<17> で使う。次数 d = 4 なので質問ごとに O(d^2 log n)。
struct Solver {
 vector<i64> ns, ans;
 explicit Solver(const vector<i64>& ns): ns(ns) {}
 void run() {
  using Z= ZMod<17>;
  ans.clear();
  for(i64 n: ns) ans.push_back(linear_recurrence<Z>({1, 1, 1, 1}, {0, 0, 0, 1}, n - 1).val());
 }
 const vector<i64>& answer() const { return ans; }
};
