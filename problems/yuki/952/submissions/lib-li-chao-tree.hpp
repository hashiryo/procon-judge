#pragma once
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// 1 つ前に閉めたドア j を「直線」にして Li Chao 木に入れ、次に閉めるドア i で
// 問い合わせる。j を固定した費用は累積和の 2 次式で、2 本の差が 1 次式になる
// ので、直線と同じく高々 1 回しか交わらない。段ごとに木を作り直す。
struct LiChaoStep {
  const Cost &w;
  int n;

  LiChaoStep(const Cost &w, int n) : w(w), n(n) {}

  void operator()(const vector<i64> &dp, vector<i64> &ndp) {
    auto f = [&](int i, int j) { return dp[j] + w(i, j); };
    LiChaoTree lct(f, 1, n + 2);
    auto tree = lct.make_tree<MINIMIZE>();
    for (int i = 1; i <= n + 1; ++i) {
      tree.insert(i - 1);
      ndp[i] = tree.query(i).first;
    }
  }
};

using Solver = DoorSolver<LiChaoStep>;
