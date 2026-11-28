#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/monge/knuth.hpp"

// NeoLibrary の knuth_dp に、費用 w(l, r) = 区間和 を渡す。dp[l][r] =
// min_{l <= k < r} dp[l][k] + dp[k+1][r] + w(l, r)、返り値は dp[0][n-1]。
// 費用が区間和なので quadrangle inequality が等号で成り立ち、Knuth の最適化が使える。
struct Solver {
  int n;
  vector<i64> pre;  // pre[i] = 先頭 i 個の和
  i64 ans = 0;

  explicit Solver(const vector<i64> &x) : n((int)x.size()), pre(n + 1) {
    for (int i = 0; i < n; ++i) pre[i + 1] = pre[i] + x[i];
  }

  void run() {
    auto w = [&](int l, int r) -> i64 { return pre[r + 1] - pre[l]; };
    ans = knuth_dp<i64>(n, w);
  }

  i64 answer() const { return ans; }
};
