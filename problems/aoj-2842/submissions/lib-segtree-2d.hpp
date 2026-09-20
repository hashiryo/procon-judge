#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"

// マージソート木。触られる座標だけを載せるので、格子が広くても点の数でしか
// メモリを使わない。矩形の和は O(log^2 N)。
struct Solver {
  int h, w;
  vector<array<int, 5>> events;
  vector<array<i64, 2>> ans;

  Solver(int h, int w, const vector<array<int, 5>> &events)
      : h(h), w(w), events(events) {}

  void run() {
    SegmentTree_2D<int, PairSum> seg(touched(events));
    ans.clear();
    for (auto &e : events) {
      int c = e[0], y = e[1], x = e[2];
      if (c == 0) seg.set(y, x, {1, 0});
      else if (c == 1) {
        if (seg.get(y, x).second) seg.set(y, x, {0, 0});
      } else if (c == 2) {
        auto [baking, done] = seg.prod(y, e[3] + 1, x, e[4] + 1);
        ans.push_back({done, baking});
      } else seg.set(y, x, {0, 1});
    }
  }

  const vector<array<i64, 2>> &answer() const { return ans; }
};
