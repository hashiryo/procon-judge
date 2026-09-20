#pragma once
#include "pj.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// b[j] ごとに「i を動かしたときの a[i-j] + b[j]」を 1 本の曲線と見て Li Chao 木
// に入れる。直線でなくてよいのは、a が凸なら曲線どうしの交点が高々 1 つに
// 定まるため。totally monotone を使う分割統治とは別の筋。
struct Solver {
  vector<i64> a, b, c;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() {
    const int n = (int)a.size(), m = (int)b.size();
    LiChaoTree lct([&](int i, int j) { return a[i - j] + b[j]; }, 0, n + m - 1);
    auto tree = lct.make_tree<MINIMIZE>();
    for (int j = 0; j < m; ++j) tree.insert(j, j, n + j);
    c.resize(n + m - 1);
    for (int i = 0; i < n + m - 1; ++i) c[i] = tree.query(i).first;
  }

  const vector<i64> &answer() const { return c; }
};
