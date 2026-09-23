#pragma once
#include "pj.hpp"
#include "mylib/optimization/monotone_minima.hpp"

// totally monotone な行列の各行の最小を分割統治で求める O((N+M) log M)。
// 行列は作らず、比較の述語だけを渡す。
struct Solver {
  vector<i64> a, b, c;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() {
    const int n = (int)a.size(), m = (int)b.size();
    auto select = [&](int i, int j, int k) {
      if (i < k) return false;
      if (i - j >= n) return true;
      return a[i - j] + b[j] >= a[i - k] + b[k];
    };
    auto r = monotone_minima(n + m - 1, m, select);
    c.resize(n + m - 1);
    for (int i = 0; i < n + m - 1; ++i) c[i] = a[i - r[i]] + b[r[i]];
  }

  const vector<i64> &answer() const { return c; }
};
