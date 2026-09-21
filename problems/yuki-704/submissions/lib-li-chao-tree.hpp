#pragma once
#include <tuple>
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// Li Chao 木。切り口 j ごとに「i を動かしたときの dp[j] + w(i, j)」を 1 本の
// 曲線として入れる。Monge であることを使わないので、問い合わせのたびに費用
// 関数を呼ぶ。
struct Solver {
  vector<i64> a, x, y;
  i64 ans = 0;

  Solver(const vector<i64> &a, const vector<i64> &x, const vector<i64> &y)
      : a(a), x(x), y(y) {}

  void run() {
    const int n = (int)a.size();
    auto w = [&](int i, int j, i64 d) { return d + pen(x[j] - a[i - 1]) + pen(y[j]); };
    LiChaoTree lct(w, 1, n + 1);
    auto tree = lct.make_tree<MINIMIZE>();
    tree.insert(0, 0);
    for (int i = 1; i < n; ++i) tree.insert(i, tree.query(i).first);
    ans = tree.query(n).first;
  }

  i64 answer() const { return ans; }
};
