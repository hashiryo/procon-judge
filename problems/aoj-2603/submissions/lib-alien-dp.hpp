#pragma once
#include "common.hpp"
#include "mylib/optimization/simplified_larsch_dp.hpp"
#include "mylib/optimization/fibonacci_search.hpp"

// 組の数の制約を罰金 p に置き換えて外し、p を凸性のある 1 変数として探索する
// (Alien DP)。内側は段を回さず 1 回の LARSCH で済むので、m に依らない。
struct Solver {
  int m;
  vector<i64> src;
  i64 ans = 0;

  Solver(int m, const vector<i64> &src) : m(m), src(src) {}

  void run() {
    const int n = (int)src.size();
    if (n <= m) {
      ans = 0;
      return;
    }
    Cost w(src);
    auto f = [&](i64 p) {
      auto wp = [&](int i, int j) { return w(i, j) + p; };
      auto dp = simplified_larsch_dp(n, wp);
      return dp[n] - p * m;
    };
    ans = fibonacci_search<MAXIMIZE>(f, -(i64)3e5, (i64)3e5).second;
  }

  i64 answer() const { return ans; }
};
