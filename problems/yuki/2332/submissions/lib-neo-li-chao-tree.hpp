#pragma once
// common.hpp も mylib の LiChaoTree を include するので、名前がぶつからないようここでは include しない。
#include <algorithm>
#include <limits>
#include "pj.hpp"
#include "mylib/string/z_algorithm.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// 接尾辞の一致は Z 配列 (lib-z-algorithm と同じ)。列を繋ぐ費用の DP は、NeoLibrary の LiChaoTree で回す。
// 位置 l から k 個足す費用 k C_l は、x = l + k の直線 C_l x + (dp[l] - l C_l) になる。
struct Solver {
  vector<int> a, b;
  vector<i64> c;
  i64 ans = 0;

  Solver(const vector<i64> &a, const vector<i64> &b, const vector<i64> &c)
      : a(a.begin(), a.end()), b(b.begin(), b.end()), c(c) {}

  void run() {
    const int n = (int)a.size(), m = (int)b.size();
    vector<int> s(a);
    s.insert(s.end(), b.begin(), b.end());
    vector<int> z = z_algorithm(s);
    LiChaoTree<i64, false> tree(0, (long long)m + n + 2);
    static constexpr i64 INF = numeric_limits<i64>::max();
    i64 dp = 0;  // dp[i]。作れなければ -1
    for (int i = 0; i < m; ++i) {
      if (dp >= 0) {
        const int len = std::min(z[n + i], n);
        tree.add_segment(i + 1, i + len + 1, c[i], dp - c[i] * i);
      }
      const i64 v = tree.query(i + 1);
      dp = v >= INF / 2 ? -1 : v;
    }
    ans = dp;
  }

  i64 answer() const { return ans; }
};
