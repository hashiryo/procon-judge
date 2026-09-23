#pragma once
#include <tuple>
#include <utility>
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// Li Chao 木。x の範囲を区間木に切って、各ノードに 1 本ずつ直線を置く。凸包を
// 保たないので追加も問い合わせも O(log C)。範囲は既定の [-2e9, 2e9)。
struct Lines {
  struct Line {
    i64 operator()(i64 x, i64 a, i64 b) const { return a * x + b; }
  };

  using Tree = LiChaoTree<Line, std::tuple<i64, i64, i64>>;

  Tree tree;
  // LiChaoTreeInterface は LiChaoTree の private な入れ子なので名前で書けない。
  decltype(std::declval<Tree &>().make_tree<MAXIMIZE>()) cht;

  Lines() : tree(Line{}) { cht = tree.make_tree<MAXIMIZE>(); }

  void insert(i64 a, i64 b) { cht.insert(a, b); }

  i64 query(i64 x) const { return cht.query(x).first; }
};

using Solver = BallSolver<Lines>;
