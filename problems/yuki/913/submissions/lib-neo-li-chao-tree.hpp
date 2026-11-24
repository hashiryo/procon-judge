#pragma once
#include "common.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTree に、区間の片方の端を 1 つの関数として載せ、もう片方の端で問う (今の Library の
// lib-li-chao-tree と同じ解き方)。分割統治の呼び出しごとに、問う側の端の範囲だけの木を作る。
struct LiChaoEngine {
  struct ByRight {
    const Cost *w;
    int r;
    i64 operator()(i64 l) const { return (*w)((int)l, r); }
  };
  struct ByLeft {
    const Cost *w;
    int l;
    i64 operator()(i64 r) const { return (*w)(l, (int)r); }
  };
  const Cost &w;

  LiChaoEngine(const Cost &w, int) : w(w) {}

  vector<i64> min_right(int L, int M, int R) {
    LiChaoTree<i64, false, ByRight> tree(L, M, R - M);
    for (int j = M + 1; j <= R; ++j) tree.add(ByRight{&w, j});
    vector<i64> v(M - L);
    for (int i = L; i < M; ++i) v[i - L] = tree.query(i);
    return v;
  }

  vector<i64> min_left(int L, int M, int R) {
    LiChaoTree<i64, false, ByLeft> tree(M + 1, R + 1, M - L + 1);
    for (int l = L; l <= M; ++l) tree.add(ByLeft{&w, l});
    vector<i64> v(R - M);
    for (int i = M; i < R; ++i) v[i - M] = tree.query(i + 1);
    return v;
  }
};

using Solver = BurnSolver<LiChaoEngine>;
