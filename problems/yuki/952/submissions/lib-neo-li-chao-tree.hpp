#pragma once
#include "common.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTreePoints に、1 つ前に閉めたドア j を 1 つの関数 dp[j] + w(i, j) として載せ、次に閉めるドア i で
// 問う (今の Library の lib-li-chao-tree と同じ解き方)。
struct LiChaoStep {
  const Cost &w;
  int n;
  vector<i64> xs;

  struct Fn {
    const Cost *w;
    int j;
    i64 d;
    i64 operator()(i64 i) const { return d + (*w)((int)i, j); }
  };

  LiChaoStep(const Cost &w, int n) : w(w), n(n), xs(n + 1) {
    for (int i = 0; i <= n; ++i) xs[i] = i + 1;
  }

  void operator()(const vector<i64> &dp, vector<i64> &ndp) {
    LiChaoTreePoints<i64, false, Fn> tree(xs, LiChaoTreePoints<i64, false, Fn>::SortedUnique{});
    for (int i = 1; i <= n + 1; ++i) {
      tree.add(Fn{&w, i - 1, dp[i - 1]});
      ndp[i] = tree.query_index(i - 1);
    }
  }
};

using Solver = DoorSolver<LiChaoStep>;
