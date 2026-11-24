#pragma once
#include <algorithm>
#include "common.hpp"
#include "_shared/lines/lichao_fn.hpp"

// 費用を展開せず、切り口 j ごとの曲線 (2 次の係数が同じなので、どの 2 本も高々 1 回しか交わらない) のまま LiChaoPointsFn に
// 載せる。今の Library の lib-li-chao-tree と同じ解き方。
struct Solver {
  struct Curve {
    i64 a, b, j, acc;
    i64 operator()(i64 i) const { return acc + b * (i - j) * (i - j - 1) / 2 - a * (i - j - 1); }
  };
  i64 a, b, w;
  vector<i64> d;
  i64 ans = 0;

  Solver(i64 a, i64 b, i64 w, const vector<i64> &d) : a(a), b(b), w(w), d(d) {}

  void run() {
    const i64 n = (i64)d.size();
    vector<i64> xs(n);
    for (i64 i = 1; i <= n; ++i) xs[i - 1] = i;
    LiChaoPointsFn<Curve> t(xs, typename LiChaoPointsFn<Curve>::SortedUnique{});
    ans = (i64)1e18;
    for (i64 i = 0; i <= n; ++i) {
      const i64 dp = i ? t.query_index((int)i - 1) + d[i - 1] : 0;
      t.add(Curve{a, b, i, dp});
      ans = std::min(ans, dp + tail(a, b, n, i));
    }
    ans += w;
  }

  i64 answer() const { return ans; }
};
