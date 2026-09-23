#pragma once
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// Li Chao 木。切り口 j ごとに「i を動かしたときの dp[j] + max(a[j..i))」を
// 1 本の曲線として入れる。Monge であることを使わないので、コスト関数を
// 毎回の問い合わせで呼ぶ。
struct Solver {
  struct RangeMax {
    using T = i64;
    static T ti() { return -(i64)1e18; }
    static T op(T l, T r) { return l > r ? l : r; }
  };

  int l;
  vector<i64> a;
  i64 ans = 0;

  Solver(int l, const vector<i64> &a) : l(l), a(a) {}

  void run() {
    const int n = (int)a.size();
    SegmentTree<RangeMax> seg(a);
    auto w = [&](int i, int j, i64 d) { return d + seg.prod(j, i); };
    LiChaoTree lct(w, 1, n + 1);
    auto tree = lct.make_tree<MAXIMIZE>();
    tree.insert(0, 0, l);
    for (int i = 1; i < n; ++i) tree.insert(i, tree.query(i).first, i + l);
    ans = tree.query(n).first;
  }

  i64 answer() const { return ans; }
};
