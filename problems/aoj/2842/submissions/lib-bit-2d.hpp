#pragma once
#include "pj.hpp"
#include "mylib/data_structure/BinaryIndexedTree_2D.hpp"

// 2 次元の BIT を焼き中と焼き上がりで 2 本持つ。格子を丸ごと確保するので、
// 触らない場所のぶんまでメモリを使うが、1 点の更新も矩形の和も O(log H log W)。
struct Solver {
  int h, w;
  vector<array<int, 5>> events;
  vector<array<i64, 2>> ans;

  Solver(int h, int w, const vector<array<int, 5>> &events)
      : h(h), w(w), events(events) {}

  void run() {
    BinaryIndexedTree_2D<i64> baking(h, w), done(h, w);
    ans.clear();
    for (auto &e : events) {
      int c = e[0], y = e[1], x = e[2];
      if (c == 0) {
        if (!baking.sum(y - 1, x - 1, y, x)) baking.add(y, x, 1);
      } else if (c == 1) {
        if (done.sum(y - 1, x - 1, y, x)) done.add(y, x, -1);
      } else if (c == 2) {
        ans.push_back({done.sum(y - 1, x - 1, e[3], e[4]),
                       baking.sum(y - 1, x - 1, e[3], e[4])});
      } else {
        if (baking.sum(y - 1, x - 1, y, x)) baking.add(y, x, -1);
        if (!done.sum(y - 1, x - 1, y, x)) done.add(y, x, 1);
      }
    }
  }

  const vector<array<i64, 2>> &answer() const { return ans; }
};
