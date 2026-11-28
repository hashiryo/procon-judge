#pragma once
// NeoLibrary の永続セグメント木 (neo/base/ds/range/PersistentSegmentTree.hpp) で rectangle-sum を解く。
// 点を x の順に並べ、y の順位を葉に持つ木に重みを積む。版 i は先頭 i 個 (x の小さい順) を入れた木で、
// x の範囲を版の範囲に、y の範囲を順位の範囲に直し、2 版の差を引く。
// 静的な問い合わせは wavelet matrix の方が速い (procon-judge の記録)。ここは差を見るための比較。
#include <algorithm>
#include <cstddef>
#include "pj.hpp"
#include "neo/base/ds/range/PersistentSegmentTree.hpp"
#include "neo/base/ds/range/segtree/Sum.hpp"

struct Solver {
  using PST = PersistentSegmentTree<monoid::Sum<i64>>;
  vector<int> xs, ys;
  vector<PST> ver;
  explicit Solver(const vector<array<int, 3>> &pts) {
    vector<array<int, 3>> p = pts;
    sort(p.begin(), p.end(), [](const auto &a, const auto &b) { return a[0] < b[0]; });
    xs.reserve(p.size()), ys.reserve(p.size());
    for (auto &e : p) xs.push_back(e[0]), ys.push_back(e[1]);
    sort(ys.begin(), ys.end());
    ys.erase(unique(ys.begin(), ys.end()), ys.end());
    PST::reserve(std::size_t(p.size()) * 20 + 1);
    ver.reserve(p.size() + 1);
    ver.push_back(PST(int(ys.size())));
    for (auto &e : p) {
      const int yr = int(lower_bound(ys.begin(), ys.end(), e[1]) - ys.begin());
      ver.push_back(ver.back().mul(yr, i64(e[2])));
    }
  }
  i64 rect_sum(int l, int d, int r, int u) const {
    const int i = int(lower_bound(xs.begin(), xs.end(), l) - xs.begin());
    const int j = int(lower_bound(xs.begin(), xs.end(), r) - xs.begin());
    const int yl = int(lower_bound(ys.begin(), ys.end(), d) - ys.begin());
    const int yu = int(lower_bound(ys.begin(), ys.end(), u) - ys.begin());
    auto sub = [](i64 b, i64 a) { return b - a; };
    return PST::prod_diff(ver[i], ver[j], yl, yu, sub);
  }
};
