#pragma once
#include "common.hpp"
#include "mylib/data_structure/SparseTable.hpp"

// 幅が 2 冪の区間の答えを段ごとに持つ。取得は重なった 2 区間の合成で済むが、
// 重なってよいのは冪等な演算のときだけ。最小値はそれに当たる。
struct Solver {
  int l;
  vector<int> a;
  vector<i64> ans;

  Solver(int l, const vector<i64> &a) : l(l), a(to_int(a)) {}

  void run() {
    SparseTable st(a, [](int x, int y) { return x < y ? x : y; });
    const int n = (int)a.size();
    vector<int> r;
    r.reserve(n - l + 1);
    for (int i = 0; i + l <= n; ++i) r.push_back(st.prod(i, i + l));
    ans = to_i64(r);
  }

  const vector<i64> &answer() const { return ans; }
};
