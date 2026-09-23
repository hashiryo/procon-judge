#pragma once
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// 段ごとに Li Chao 木を作り直して、切り口 j を 1 本の曲線として入れる。
// totally monotone を使わないので、段ごとに O(N log N) の問い合わせになる。
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
    vector<i64> dp(n + 1, (i64)1e9);
    dp[0] = 0;
    for (int t = m; t--;) {
      auto f = [&](int i, int j) { return dp[j] + w(i, j); };
      LiChaoTree lct(f, 1, n + 1);
      auto tree = lct.make_tree<MINIMIZE>();
      vector<i64> ndp(n + 1);
      for (int i = 1; i <= n; ++i) {
        tree.insert(i - 1);
        ndp[i] = tree.query(i).first;
      }
      ndp[0] = 0, dp = ndp;
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
