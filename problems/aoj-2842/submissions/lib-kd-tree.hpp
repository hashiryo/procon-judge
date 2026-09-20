#pragma once
#include "common.hpp"
#include "mylib/data_structure/KDTree.hpp"

// kd 木。点 1 つにつき 1 ノードで済むが、矩形の取得は最悪 O(sqrt N) になる。
struct Solver {
  int h, w;
  vector<array<int, 5>> events;
  vector<array<i64, 2>> ans;

  Solver(int h, int w, const vector<array<int, 5>> &events)
      : h(h), w(w), events(events) {}

  void run() {
    KDTree<int, 2, PairSum> kdt(touched(events));
    ans.clear();
    for (auto &e : events) {
      int c = e[0], y = e[1], x = e[2];
      if (c == 0) kdt.set(y, x, {1, 0});
      else if (c == 1) {
        if (kdt.get(y, x).second) kdt.set(y, x, {0, 0});
      } else if (c == 2) {
        // kd 木の側は閉区間で受ける。
        auto [baking, done] = kdt.prod_cuboid(y, e[3], x, e[4]);
        ans.push_back({done, baking});
      } else kdt.set(y, x, {0, 1});
    }
  }

  const vector<array<i64, 2>> &answer() const { return ans; }
};
