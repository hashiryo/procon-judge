#pragma once
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 右から左へ降りる向き (min_left)。同じことを鏡写しにやるので、載せるのは
// 区間最小と区間加算になる。木を降りる向きが変わるぶん、枝の辿り方が変わる。
struct Solver {
  struct MinAdd {
    using T = i64;
    using E = i64;
    static T ti() { return 1LL << 60; }
    static T op(T l, T r) { return l < r ? l : r; }
    static void mp(T &v, E y) { v += y; }
    static void cp(E &x, E y) { x += y; }
  };

  int n;
  vector<array<int, 3>> qs;
  vector<i64> ans;

  Solver(int n, const vector<array<int, 3>> &qs) : n(n), qs(qs) {}

  void run() {
    SegmentTree<MinAdd> a(n + 1, [](int i) { return (i64)i; });
    SegmentTree<MinAdd> b(n + 1, [](int i) { return (i64)i; });
    for (int q = (int)qs.size(); q--;) {
      auto [x, d, l] = qs[q];
      if (d == 1) {
        int i = a.min_left(n + 1, [&](i64 v) { return v > x; });
        b.apply(0, i, -2LL * l);
      } else {
        int i = b.min_left(n + 1, [&](i64 v) { return v > x; });
        a.apply(i, n + 1, 2LL * l);
      }
    }
    ans.resize(n);
    for (int i = 1; i <= n; ++i) ans[i - 1] = (a[i] - b[i]) / 2;
  }

  const vector<i64> &answer() const { return ans; }
};
