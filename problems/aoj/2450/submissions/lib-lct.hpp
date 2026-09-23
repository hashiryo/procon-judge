#pragma once
#include <algorithm>
#include "pj.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// Link-Cut 木。道の積を expose だけで出せるので、区間への分解が要らない。
// 長さが作用の計算に要るので、節点の値に size を持たせる。
struct Solver {
  struct MaxSum {
    static inline i64 INF = 1LL << 60;
    struct T {
      i64 sum, max, lmax, rmax;
      int size;
    };
    using E = i64;
    static T op(const T &a, const T &b) {
      return {a.sum + b.sum, std::max({a.max, b.max, a.rmax + b.lmax}),
              std::max(a.lmax, a.sum + b.lmax), std::max(a.rmax + b.sum, b.rmax),
              a.size + b.size};
    }
    static void mp(T &v, const E &f) {
      v.sum = f * v.size;
      v.max = v.lmax = v.rmax = std::max(v.sum, f);
    }
    static void cp(E &pre, const E &suf) { pre = suf; }
  };

  LinkCutTree<MaxSum> lct;

  Solver(int n, const vector<i64> &w, const vector<array<int, 2>> &edges)
      : lct(n) {
    for (int i = 0; i < n; ++i) lct.set(i, {w[i], w[i], w[i], w[i], 1});
    for (auto &e : edges) lct.link(e[0], e[1]);
  }

  void assign(int u, int v, i64 c) { lct.apply(u, v, c); }

  i64 max_sum(int u, int v) { return lct.prod(u, v).max; }
};
