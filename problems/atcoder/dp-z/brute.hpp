#pragma once
#include <algorithm>
#include <climits>
#include "pj.hpp"

// 愚直解。dp_i = min_{j<i} dp_j + (h_i - h_j)^2 + C を O(N^2) で求める。
struct Solver {
  i64 c;
  vector<i64> h;
  i64 ans = 0;

  Solver(i64 c, const vector<i64> &h) : c(c), h(h) {}

  void run() {
    const int n = (int)h.size();
    vector<i64> dp(n, 0);
    for (int i = 1; i < n; ++i) {
      dp[i] = LLONG_MAX;
      for (int j = 0; j < i; ++j) dp[i] = std::min(dp[i], dp[j] + (h[i] - h[j]) * (h[i] - h[j]) + c);
    }
    ans = dp[n - 1];
  }

  i64 answer() const { return ans; }
};
