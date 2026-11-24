#pragma once
#include <algorithm>
#include <climits>
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTreePoints を最大で使い、切り口 j ごとの「i を動かしたときの dp[j] + max(a[j..i))」を 1 つの関数と
// して、i ≥ j + L の区間にだけ載せる (今の Library の lib-li-chao-tree と同じ解き方)。区間の最大は今の Library の
// SegmentTree のまま。
struct Solver {
  struct RangeMax {
    using T = i64;
    static T ti() { return -(i64)1e18; }
    static T op(T l, T r) { return l > r ? l : r; }
  };
  struct Fn {
    SegmentTree<RangeMax> *seg;  // prod が const でないので、const を付けずに持つ
    int j;
    i64 d;
    i64 operator()(i64 i) const { return d + seg->prod(j, (int)i); }
  };

  int l;
  vector<i64> a;
  i64 ans = 0;

  Solver(int l, const vector<i64> &a) : l(l), a(a) {}

  void run() {
    const int n = (int)a.size();
    SegmentTree<RangeMax> seg(a);
    vector<i64> xs(n);
    for (int i = 0; i < n; ++i) xs[i] = i + 1;
    LiChaoTreePoints<i64, true, Fn> t(std::move(xs), LiChaoTreePoints<i64, true, Fn>::SortedUnique{});
    // x = i は添字 i - 1。x in [j + l, n + 1) は添字 [j + l - 1, n)。
    t.add_index(Fn{&seg, 0, 0}, std::min(l, n + 1) - 1, n);
    // 関数が 1 つも載っていない点 (どの切り口からも届かない i) は 0 として扱う (今の Library の Li Chao 木が返す値と同じ)。
    auto get = [&](int i) {
      const i64 v = t.query_index(i - 1);
      return v == LLONG_MIN ? 0 : v;
    };
    for (int i = 1; i < n; ++i) t.add_index(Fn{&seg, i, get(i)}, std::min(i + l, n + 1) - 1, n);
    ans = get(n);
  }

  i64 answer() const { return ans; }
};
