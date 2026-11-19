#pragma once
#include <tuple>
#include <utility>
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// 今の Library の Li Chao 木に直線を入れる。範囲は問題文の制約の [1, 10^6]。
struct Lines {
  struct Line {
    i64 operator()(i64 x, i64 a, i64 b) const { return a * x + b; }
  };
  using Tree = LiChaoTree<Line, std::tuple<i64, i64, i64>>;
  Tree tree;
  decltype(std::declval<Tree &>().template make_tree<MINIMIZE>()) cht;

  Lines() : tree(Line{}, 1, 1000001) { cht = tree.make_tree<MINIMIZE>(); }
  void insert(i64 a, i64 b) { cht.insert(a, b); }
  i64 query(i64 x) { return cht.query(x).first; }
};

using Solver = FrogSolver<Lines>;
