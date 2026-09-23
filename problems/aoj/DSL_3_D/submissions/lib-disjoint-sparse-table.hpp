#pragma once
#include "common.hpp"
#include "mylib/data_structure/DisjointSparseTable.hpp"

// 段ごとに中央から左右へ畳んだ累積を持つ。取得は重ならない 2 区間の合成に
// なるので、冪等でない演算にも使える。そのぶん表の作りは重い。
struct Solver {
  int l;
  vector<int> a;
  vector<i64> ans;

  Solver(int l, const vector<i64> &a) : l(l), a(to_int(a)) {}

  void run() {
    DisjointSparseTable<int> dst(a, [](int x, int y) { return x < y ? x : y; });
    const int n = (int)a.size();
    vector<int> r;
    r.reserve(n - l + 1);
    for (int i = 0; i + l <= n; ++i) r.push_back(dst.prod(i, i + l));
    ans = to_i64(r);
  }

  const vector<i64> &answer() const { return ans; }
};
