#pragma once
// ライブラリを使う提出が共有するもの。解法はここに 1 つ置いて、提出は座標の
// 型を選ぶだけにする。
#include <algorithm>
#include <type_traits>
#include <utility>
#include "pj.hpp"
#include "mylib/geometry/Polygon.hpp"
#include "mylib/data_structure/RangeSet.hpp"

template <class R> struct GeoSolver {
  using P = geo::Point<R>;
  using L = geo::Line<R>;

  // 格子の範囲。問題の座標がこの内側に収まる。
  static constexpr int LO = -2001, HI = 2001;

  vector<array<i64, 2>> src;
  i64 ans = 0;

  explicit GeoSolver(const vector<array<i64, 2>> &src) : src(src) {}

  // 浮動小数点数のときだけ許容誤差つきの丸めを使う。有理数なら正確に出る。
  static i64 ceil_i(const R &x) {
    if constexpr (std::is_floating_point_v<R>) return (i64)geo::err_ceil(x);
    else return (i64)ceil(x);
  }
  static i64 floor_i(const R &x) {
    if constexpr (std::is_floating_point_v<R>) return (i64)geo::err_floor(x);
    else return (i64)floor(x);
  }

  void run() {
    vector<P> ps(src.size());
    for (size_t i = 0; i < src.size(); ++i) ps[i] = P{R(src[i][0]), R(src[i][1])};
    geo::Polygon<R> g(ps);
    P dir{R(1), R(0)};
    ans = 0;
    // 高さ 1 の帯ごとに、多角形が覆う格子の列を数える。
    for (int y = LO; y < HI; ++y) {
      L lower{P{R(0), R(y)}, dir}, upper{P{R(0), R(y + 1)}, dir};
      vector<R> xd, xu;
      for (auto &e : g.edges()) {
        auto cd = geo::cross_points(lower, e);
        auto cu = geo::cross_points(upper, e);
        if (cd.size() != 1 || cu.size() != 1) continue;
        xd.push_back(cd[0].x), xu.push_back(cu[0].x);
      }
      std::sort(xd.begin(), xd.end()), std::sort(xu.begin(), xu.end());
      RangeSet<int> rs;
      const int m = (int)xd.size();
      int lo = LO;
      for (int i = 0; i < m; ++i) {
        if (i & 1) {
          int hi = (int)ceil_i(std::max(xd[i], xu[i]));
          if (lo < hi) rs.insert(lo, hi - 1);
        } else lo = (int)floor_i(std::min(xd[i], xu[i]));
      }
      for (auto [a, b] : rs) ans += b - a + 1;
    }
  }

  i64 answer() const { return ans; }
};
