#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/bostan_mori.hpp"

// 線形漸化式の第 n 項を Bostan-Mori で出す。次数 d = 4 なので質問ごとに
// O(d^2 log n)。前計算は無い。
struct Solver {
  vector<i64> ns, ans;

  explicit Solver(const vector<i64> &ns) : ns(ns) {}

  void run() {
    using Mint = ModInt<17>;
    ans.clear();
    for (i64 n : ns)
      ans.push_back(linear_recurrence<Mint>({1, 1, 1, 1}, {0, 0, 0, 1}, n - 1).val());
  }

  const vector<i64> &answer() const { return ans; }
};
