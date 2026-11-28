#pragma once
#include <tuple>
#include "pj.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// 今の Library の Li Chao 木に直線 y = f_j X + dp[j] を足しながら、点 s_i での最小を引く。
// x の範囲を区間木に切り、各節に 1 本ずつ直線を置く。土台の比較用。
struct Solver {
  struct Line {
    i64 operator()(i64 xx, i64 a, i64 b) const { return a * xx + b; }
  };
  using Tree = LiChaoTree<Line, std::tuple<i64, i64, i64>>;

  int n;
  i64 x;
  vector<i64> s, f;
  Tree tree;
  decltype(std::declval<Tree &>().make_tree<MINIMIZE>()) cht;
  i64 ans = 0;

  Solver(const vector<i64> &s, const vector<i64> &f, i64 x)
      : n((int)s.size()), x(x), s(s), f(f), tree(Line{}, (i64)0, (i64)1000001) {
    cht = tree.make_tree<MINIMIZE>();
  }

  void run() {
    cht.insert(x, 0);  // まだ 1 体も倒していない状態
    i64 last = 0;
    for (int i = 0; i < n; ++i) {
      last = cht.query(s[i]).first;
      cht.insert(f[i], last);
    }
    ans = last;  // dp[n]
  }

  i64 answer() const { return ans; }
};
