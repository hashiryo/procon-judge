#pragma once
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree.hpp"
#include "mylib/optimization/simplified_larsch_dp.hpp"

// コスト行列が concave Monge であることを使う LARSCH。行を 1 つずつ確定
// させながら降りるので、コスト関数を O(N log N) 回しか呼ばない。
// 最大化を最小化に直すため、値の符号を反転させて区間最小で引く。
struct Solver {
  struct RangeMin {
    using T = i64;
    static T ti() { return (i64)1e18; }
    static T op(T l, T r) { return l < r ? l : r; }
  };

  int l;
  vector<i64> a;
  i64 ans = 0;

  Solver(int l, const vector<i64> &a) : l(l), a(a) {}

  void run() {
    const int n = (int)a.size();
    vector<i64> neg(n);
    for (int i = 0; i < n; ++i) neg[i] = -a[i];
    SegmentTree<RangeMin> seg(neg);
    auto w = [&](int i, int j) -> i64 {
      if (i - j < l) return (i64)1e18;  // 短すぎる区間は選べない
      return seg.prod(j, i);
    };
    ans = -simplified_larsch_dp(n, w)[n];
  }

  i64 answer() const { return ans; }
};
