#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTreePoints に、j ごとの関数 a[i - j] + b[j] を i in [j, n + j) の区間に載せる (今の Library の
// lib-li-chao-tree と同じ解き方)。a が凸なので、どの 2 つも高々 1 回しか交わらない。
struct Solver {
  vector<i64> a, b, c;

  struct Fn {
    const i64 *a;
    int j;
    i64 bj;
    i64 operator()(i64 i) const { return a[i - j] + bj; }
  };

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() {
    const int n = (int)a.size(), m = (int)b.size();
    vector<i64> xs(n + m - 1);
    for (int i = 0; i < n + m - 1; ++i) xs[i] = i;
    LiChaoTreePoints<i64, false, Fn> t(std::move(xs), LiChaoTreePoints<i64, false, Fn>::SortedUnique{});
    for (int j = 0; j < m; ++j) t.add_index(Fn{a.data(), j, b[j]}, j, n + j);
    c.resize(n + m - 1);
    for (int i = 0; i < n + m - 1; ++i) c[i] = t.query_index(i);
  }

  const vector<i64> &answer() const { return c; }
};
