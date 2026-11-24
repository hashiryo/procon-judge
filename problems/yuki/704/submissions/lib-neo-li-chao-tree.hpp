#pragma once
#include "common.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTreePoints に、切り口 j ごとの「i を動かしたときの dp[j] + w(i, j)」を 1 つの関数として載せる
// (今の Library の lib-li-chao-tree と同じ解き方)。問いの x は 1, 2, ..., N なので、点の添字で引く。
struct Solver {
  vector<i64> a, x, y;
  i64 ans = 0;

  struct Fn {
    const Solver *s;
    int j;
    i64 d;
    i64 operator()(i64 i) const { return d + pen(s->x[j] - s->a[i - 1]) + pen(s->y[j]); }
  };

  Solver(const vector<i64> &a, const vector<i64> &x, const vector<i64> &y) : a(a), x(x), y(y) {}

  void run() {
    const int n = (int)a.size();
    vector<i64> xs(n);
    for (int i = 0; i < n; ++i) xs[i] = i + 1;
    LiChaoTreePoints<i64, false, Fn> t(std::move(xs), LiChaoTreePoints<i64, false, Fn>::SortedUnique{});
    t.add(Fn{this, 0, 0});
    for (int i = 1; i < n; ++i) t.add(Fn{this, i, t.query_index(i - 1)});
    ans = t.query_index(n - 1);
  }

  i64 answer() const { return ans; }
};
