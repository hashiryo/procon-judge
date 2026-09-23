#pragma once
#include <tuple>
#include <utility>
#include "pj.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// Li Chao 木。x の範囲を区間木に切って、各ノードに 1 本ずつ直線を置く。
// 凸包を保たないので、追加も取得も O(log C) で揃う。
struct Solver {
  struct Line {
    i64 operator()(i64 x, i64 a, i64 b) const { return a * x + b; }
  };

  using Tree = LiChaoTree<Line, std::tuple<i64, i64, i64>>;

  Tree tree;
  // LiChaoTreeInterface は LiChaoTree の private な入れ子なので名前で書けない。
  decltype(std::declval<Tree &>().template make_tree<MINIMIZE>()) cht;

  explicit Solver(const vector<array<i64, 2>> &lines) : tree(Line{}) {
    cht = tree.make_tree<MINIMIZE>();
    for (auto &e : lines) cht.insert(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { cht.insert(a, b); }

  i64 query(i64 p) { return cht.query(p).first; }
};
