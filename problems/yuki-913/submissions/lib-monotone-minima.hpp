#pragma once
#include "common.hpp"
#include "mylib/optimization/monotone_minima.hpp"

// 片方の端を行、もう片方の端を列にした費用の行列は totally monotone (行の最小を
// 取る列が単調) なので、行ごとの最小の位置を分割統治で O((H + W) log H) で取る。
// 木は作らず、費用を直接評価する。
struct MonotoneEngine {
  const Cost &w;

  MonotoneEngine(const Cost &w, int) : w(w) {}

  vector<i64> min_right(int L, int M, int R) {
    // 行 i in [L, M)、列 r in [M, R)、値 w(i, r + 1)。
    auto select = [&](int i, int j, int k) {
      return w(L + i, M + 1 + j) > w(L + i, M + 1 + k);
    };
    auto id = monotone_minima(M - L, R - M, select);
    vector<i64> v(M - L);
    for (int i = L; i < M; ++i) v[i - L] = w(i, M + 1 + id[i - L]);
    return v;
  }

  vector<i64> min_left(int L, int M, int R) {
    // 行 i in [M, R)、列 l in [L, M]、値 w(l, i + 1)。
    auto select = [&](int i, int j, int k) {
      return w(L + j, M + 1 + i) > w(L + k, M + 1 + i);
    };
    auto id = monotone_minima(R - M, M + 1 - L, select);
    vector<i64> v(R - M);
    for (int i = M; i < R; ++i) v[i - M] = w(L + id[i - M], i + 1);
    return v;
  }
};

using Solver = BurnSolver<MonotoneEngine>;
