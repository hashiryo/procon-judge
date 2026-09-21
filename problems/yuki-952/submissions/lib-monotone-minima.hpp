#pragma once
#include "common.hpp"
#include "mylib/optimization/monotone_minima.hpp"

// 次に閉めるドア i を行、1 つ前に閉めたドア j を列にした費用の行列は totally
// monotone (行の最小を取る列が単調) なので、行ごとの最小の位置を分割統治で取る。
// j >= i には遷移が無いので INF で埋める。木は作らず、費用を直接評価する。
struct MonotoneStep {
  const Cost &w;
  int n;

  MonotoneStep(const Cost &w, int n) : w(w), n(n) {}

  void operator()(const vector<i64> &dp, vector<i64> &ndp) {
    auto cost = [&](int i, int j) { return j < i ? dp[j] + w(i, j) : INF; };
    auto select = [&](int i, int j, int k) { return cost(i, j) > cost(i, k); };
    auto id = monotone_minima(n + 2, n + 1, select);
    for (int i = 1; i <= n + 1; ++i) ndp[i] = cost(i, id[i]);
  }
};

using Solver = DoorSolver<MonotoneStep>;
