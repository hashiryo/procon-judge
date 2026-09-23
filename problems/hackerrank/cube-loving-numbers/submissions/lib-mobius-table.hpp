#pragma once
#include "pj.hpp"
#include "mylib/number_theory/tables.hpp"

// メビウス関数の表を 1e6 まで作り、x^3 <= N の範囲で -mu(x) floor(N / x^3) を足す。
// 表は全テストケースで共有する。
struct Solver {
  vector<i64> ns, ans;

  explicit Solver(const vector<i64> &ns) : ns(ns) {}

  void run() {
    auto mu = mobius_table(1'000'010);
    ans.clear();
    for (i64 n : ns) {
      i64 cnt = 0;
      for (i64 x = 2; x * x * x <= n; x++) cnt -= n / (x * x * x) * mu[x];
      ans.push_back(cnt);
    }
  }

  const vector<i64> &answer() const { return ans; }
};
