#pragma once
// ライブラリを使う提出が共有するもの。解法はここに 1 つ置いて、提出は座標の
// 型を選ぶだけにする。
#include <algorithm>
#include <utility>
#include "pj.hpp"
#include "mylib/geometry/Segment.hpp"

template <class R> struct GeoSolver {
  using P = geo::Point<R>;
  using S = geo::Segment<R>;

  array<i64, 4> ab;
  vector<array<i64, 6>> src;
  i64 ans = 0;

  GeoSolver(const array<i64, 4> &ab, const vector<array<i64, 6>> &src)
      : ab(ab), src(src) {}

  void run() {
    S line;
    line.p = P{R(ab[0]), R(ab[1])}, line.q = P{R(ab[2]), R(ab[3])};
    // AB と交わる線分について、交点と「その線を跨ぐと向きが変わるか」を持つ。
    vector<pair<P, bool>> cps;
    for (auto &e : src) {
      S s;
      s.p = P{R(e[0]), R(e[1])}, s.q = P{R(e[2]), R(e[3])};
      auto ps = geo::cross_points(line, s);
      if (ps.size()) cps.emplace_back(ps[0], 1 ^ (bool)e[4] ^ (bool)e[5]);
    }
    if (cps.empty()) {
      ans = 0;
      return;
    }
    // A から B へ辿って、向きが変わる回数を数える。
    std::sort(cps.begin(), cps.end());
    bool cur = cps[0].second;
    ans = 0;
    for (size_t i = 1; i < cps.size(); ++i)
      if (cur ^ cps[i].second) ++ans, cur = !cur;
  }

  i64 answer() const { return ans; }
};
