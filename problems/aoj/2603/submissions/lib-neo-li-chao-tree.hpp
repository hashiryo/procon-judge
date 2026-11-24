#pragma once
#include "common.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTreePoints を段ごとに作り直し、切り口 j を 1 つの関数 dp[j] + w(i, j) として載せる (今の Library の
// lib-li-chao-tree と同じ解き方)。
struct Solver {
  int m;
  vector<i64> src;
  i64 ans = 0;

  struct Fn {
    const Cost *w;
    int j;
    i64 d;
    i64 operator()(i64 i) const { return d + (*w)((int)i, j); }
  };

  Solver(int m, const vector<i64> &src) : m(m), src(src) {}

  void run() {
    const int n = (int)src.size();
    if (n <= m) {
      ans = 0;
      return;
    }
    Cost w(src);
    vector<i64> dp(n + 1, (i64)1e9), xs(n);
    for (int i = 0; i < n; ++i) xs[i] = i + 1;
    dp[0] = 0;
    for (int t = m; t--;) {
      LiChaoTreePoints<i64, false, Fn> tree(xs, LiChaoTreePoints<i64, false, Fn>::SortedUnique{});
      vector<i64> ndp(n + 1);
      for (int i = 1; i <= n; ++i) {
        tree.add(Fn{&w, i - 1, dp[i - 1]});
        ndp[i] = tree.query_index(i - 1);
      }
      ndp[0] = 0, dp = ndp;
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
