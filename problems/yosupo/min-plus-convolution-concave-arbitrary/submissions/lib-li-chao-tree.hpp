#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// 列 j の関数 g_j(k) = a[k-j] + b[j] を、有効な k の範囲 [j, j+n) にだけ Li Chao 木に載せる。
// g_j と g_j' は高々 1 回しか交わらない (a が凹なので、差が k について単調) ので、擬直線として扱える。
struct ConcaveFn {
  const i64 *a = nullptr;
  int j = 0;
  i64 b = 0;
  i64 operator()(long long k) const { return a ? a[k - j] + b : numeric_limits<i64>::max(); }
  static ConcaveFn none() { return {}; }
};

struct Solver {
  vector<i64> a, b, c;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() {
    const int n = (int)a.size(), m = (int)b.size(), h = n + m - 1;
    LiChaoTree<i64, false, ConcaveFn> t(0, h + 1, n + 1);
    for (int j = 0; j < m; ++j) t.add(ConcaveFn{a.data(), j, b[j]}, j, j + n);
    c.resize(h);
    for (int k = 0; k < h; ++k) c[k] = t.query(k);
  }

  const vector<i64> &answer() const { return c; }
};
