#pragma once
// ライブラリを使う提出が共有するもの。解法はここに 1 つ置いて、提出は座標の
// 型を選ぶだけにする。
#include <utility>
#include "pj.hpp"
#include "mylib/geometry/Segment.hpp"

template <class R> struct GeoSolver {
  using P = geo::Point<R>;
  using S = geo::Segment<R>;

  vector<array<i64, 4>> src;
  i64 ans = 0;

  explicit GeoSolver(const vector<array<i64, 4>> &src) : src(src) {}

  void run() {
    // 線分を 1 本ずつ足していき、増える領域の数を数える。新しい線分が
    // 既存の線分と交わるたびに 1 つ増える。同じ点での重複は数えない。
    ans = 1;
    vector<S> ss;
    for (auto &e : src) {
      S s;
      s.p = P{R(e[0]), R(e[1])}, s.q = P{R(e[2]), R(e[3])};
      ++ans;
      vector<P> ps{s.p, s.q};
      for (auto &t : ss) {
        auto cp = geo::cross_points(s, t);
        if (!cp.size()) continue;
        bool fresh = true;
        for (auto &p : ps) fresh &= cp[0] != p;
        if (fresh) ++ans, ps.push_back(cp[0]);
      }
      ss.push_back(s);
    }
  }

  i64 answer() const { return ans; }
};
