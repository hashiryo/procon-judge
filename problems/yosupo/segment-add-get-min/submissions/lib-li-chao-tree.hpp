#pragma once
#include <climits>
#include <tuple>
#include <utility>
#include "pj.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// 今の Library の Li Chao 木に、区間つきで直線を入れる。範囲は既定の [-2e9, 2e9)。
struct Solver {
  struct Line {
    i64 operator()(i64 x, i64 a, i64 b) const { return a * x + b; }
  };

  using Tree = LiChaoTree<Line, std::tuple<i64, i64, i64>>;

  Tree tree;
  // LiChaoTreeInterface は LiChaoTree の private な入れ子なので名前で書けない。
  decltype(std::declval<Tree &>().template make_tree<MINIMIZE>()) cht;

  explicit Solver(const vector<array<i64, 4>> &segs) : tree(Line{}) {
    cht = tree.make_tree<MINIMIZE>();
    for (auto &e : segs) cht.insert(e[2], e[3], e[0], e[1]);
  }

  void insert(i64 l, i64 r, i64 a, i64 b) { cht.insert(a, b, l, r); }

  i64 query(i64 p) {
    auto [v, id] = cht.query(p);
    return id == -1 ? LLONG_MAX : v;
  }
};
