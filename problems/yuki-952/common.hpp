#pragma once
// ライブラリを使う提出が共有するもの。閉めるドアを 1 つずつ増やしていく DP の
// 骨組みはどの実装でも同じなので、ここに置く。比べたいのは 1 段ぶんの遷移
// ndp[i] = min_j dp[j] + w(i, j) を何で取るか。
#include <algorithm>
#include "pj.hpp"

constexpr i64 INF = 1e17;

// 閉めたドアの位置を左から順に決めていく。1 つ前に閉めたのが j で次に閉めるのが
// i なら、間のドア j+1 .. i-1 が 1 つの連結成分になり、危険度は
// (A_{j+1} + ... + A_{i-1})^2 になる。位置 0 と N+1 は両端の外に置いた仮のドア。
struct Cost {
  vector<i64> sum;  // sum[i] = A_1 + ... + A_i

  explicit Cost(const vector<i64> &a) : sum(a.size() + 1, 0) {
    for (size_t i = 0; i < a.size(); ++i) sum[i + 1] = sum[i] + a[i];
  }

  // 1 <= i <= N+1, 0 <= j < i
  i64 operator()(int i, int j) const {
    const i64 d = sum[i - 1] - sum[j];
    return d * d;
  }
};

// Step は次を実装する。
//   Step(const Cost &w, int n);
//   // 1 段進める。i in [1, n+1] について ndp[i] = min_{0 <= j < i} dp[j] + w(i, j)。
//   // ndp[0] は触らなくてよい。
//   void operator()(const vector<i64> &dp, vector<i64> &ndp);
template <class Step> struct DoorSolver {
  vector<i64> a, ans;

  explicit DoorSolver(const vector<i64> &a) : a(a) {}

  void run() {
    const int n = (int)a.size();
    Cost w(a);
    Step step(w, n);
    vector<i64> dp(n + 2, INF), ndp(n + 2, INF);
    dp[0] = 0;
    ans.assign(n, 0);
    // t 段目のあとの dp[n+1] は、右端の外の仮のドアを含めて t 個閉めた形。閉めた
    // 本物のドアは t-1 個なので、開いているドアは n-t+1 個。
    for (int t = 1; t <= n; ++t) {
      step(dp, ndp);
      ndp[0] = INF;
      std::swap(dp, ndp);
      ans[n - t] = dp[n + 1];
    }
  }

  const vector<i64> &answer() const { return ans; }
};
