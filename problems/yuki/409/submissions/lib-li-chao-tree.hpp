#pragma once
#include <algorithm>
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// Li Chao 木。Monge であることを使わず、切り口ごとの曲線として入れる。
struct Solver {
  i64 a, b, w;
  vector<i64> d;
  i64 ans = 0;

  Solver(i64 a, i64 b, i64 w, const vector<i64> &d) : a(a), b(b), w(w), d(d) {}

  void run() {
    const i64 n = (i64)d.size();
    auto cost = [&](int i, int j, i64 acc) {
      return acc + d[i - 1] + b * (i - j) * (i - j - 1) / 2 - a * (i - j - 1);
    };
    LiChaoTree lct(cost, 1, (int)n + 1);
    auto tree = lct.make_tree<MINIMIZE>();
    ans = (i64)1e18;
    for (i64 i = 0; i <= n; ++i) {
      i64 dp = i ? tree.query((int)i).first : 0;
      tree.insert((int)i, dp);
      ans = std::min(ans, dp + tail(a, b, n, i));
    }
    ans += w;
  }

  i64 answer() const { return ans; }
};
