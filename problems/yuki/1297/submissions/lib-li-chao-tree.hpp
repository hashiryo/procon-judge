#pragma once
#include <tuple>
#include <utility>
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// Li Chao 木。x の範囲を区間木に切って、各ノードに 1 本ずつ直線を置く。凸包を
// 保たないので追加も問い合わせも O(log C)。範囲の端では a x が 64 bit を超える
// ので、評価だけ 128 bit で行う。
struct Lines {
  struct Line {
    __int128 operator()(i64 x, i64 a, i64 b) const { return (__int128)a * x + b; }
  };

  using Tree = LiChaoTree<Line, std::tuple<i64, i64, i64>>;

  Tree tree;
  // LiChaoTreeInterface は LiChaoTree の private な入れ子なので名前で書けない。
  decltype(std::declval<Tree &>().make_tree<MINIMIZE>()) cht;

  Lines() : tree(Line{}, (i64)-1e17, (i64)1e17) { cht = tree.make_tree<MINIMIZE>(); }

  void insert(i64 a, i64 b) { cht.insert(a, b); }

  i64 query(i64 x) const { return (i64)cht.query(x).first; }
};

using Solver = StatueSolver<Lines>;
