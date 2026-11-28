#pragma once
// NeoLibrary の永続 Li Chao 木 (neo/algo/optimize/line/PersistentLiChaoTree.hpp) を使う。
// 版 v から add_segment した新しい版を、現れる順に ver に積む。
#include <cstddef>
#include "pj.hpp"
#include "neo/algo/optimize/line/PersistentLiChaoTree.hpp"

struct Solver {
  using P = PersistentLiChaoTree<i64>;
  vector<P> ver;
  explicit Solver(int q) {
    // 線分 1 本は最悪 log^2 (座標の幅 2e6 で 21 * 21) 節を写す。
    P::reserve(std::size_t(q) * 500 + 1);
    ver.reserve(q + 1);
    ver.push_back(P(-1000000, 1000001));
  }
  void insert(int v, i64 l, i64 r, i64 a, i64 b) { ver.push_back(ver[v].add_segment(l, r, a, b)); }
  i64 query(int v, i64 x) const { return ver[v].query(x); }
};
