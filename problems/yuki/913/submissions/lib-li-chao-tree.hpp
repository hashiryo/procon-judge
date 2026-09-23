#pragma once
#include <tuple>
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// 区間の片方の端を「直線」として Li Chao 木に入れ、もう片方の端で問い合わせる。
// 端を 1 つ固定した悲しさは反対側の端の 2 次式で、2 本の差が 1 次式になるので、
// 直線と同じく高々 1 回しか交わらない。追加も問い合わせも O(log N)。
struct LiChaoEngine {
  // 右端を直線にして左端で問い合わせる木と、左端を直線にして右端で問い合わせる木。
  struct ByRight {
    const Cost *w;
    i64 operator()(int l, int r) const { return (*w)(l, r); }
  };
  struct ByLeft {
    const Cost *w;
    i64 operator()(int r, int l) const { return (*w)(l, r); }
  };

  LiChaoTree<ByRight, std::tuple<int, int>> by_right;
  LiChaoTree<ByLeft, std::tuple<int, int>> by_left;

  LiChaoEngine(const Cost &w, int n)
      : by_right(ByRight{&w}, 0, n + 1), by_left(ByLeft{&w}, 0, n + 1) {}

  vector<i64> min_right(int L, int M, int R) {
    auto tree = by_right.make_tree<MINIMIZE>();
    for (int j = M + 1; j <= R; ++j) tree.insert(j);
    vector<i64> v(M - L);
    for (int i = L; i < M; ++i) v[i - L] = tree.query(i).first;
    return v;
  }

  vector<i64> min_left(int L, int M, int R) {
    auto tree = by_left.make_tree<MINIMIZE>();
    for (int l = L; l <= M; ++l) tree.insert(l);
    vector<i64> v(R - M);
    for (int i = M; i < R; ++i) v[i - M] = tree.query(i + 1).first;
    return v;
  }
};

using Solver = BurnSolver<LiChaoEngine>;
