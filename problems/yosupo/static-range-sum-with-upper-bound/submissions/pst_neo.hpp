#pragma once
// NeoLibrary の永続セグメント木 (neo/base/ds/range/PersistentSegmentTree.hpp) で static-range-sum-with-upper-bound を解く。
// 値の順位を葉に持ち、葉に (個数, 値の和) を積む。版 i は先頭 i 個を入れた木で、x 以下の順位までを 2 版の差で引く。
// 静的な問い合わせは wavelet matrix の方が速い (procon-judge の記録)。ここは差を見るための比較。
#include <algorithm>
#include <cstddef>
#include "pj.hpp"
#include "neo/base/ds/range/PersistentSegmentTree.hpp"

struct CountSumM {
  struct T {
    int c;
    i64 s;
  };
  static T ti() { return {0, 0}; }
  static T op(T a, T b) { return {a.c + b.c, a.s + b.s}; }
};

struct Solver {
  using PST = PersistentSegmentTree<CountSumM>;
  vector<int> vals;
  vector<PST> ver;
  explicit Solver(const vector<int> &a) : vals(a) {
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
    PST::reserve(std::size_t(a.size()) * 20 + 1);
    ver.reserve(a.size() + 1);
    ver.push_back(PST(int(vals.size())));
    for (int x : a) {
      const int rank = int(lower_bound(vals.begin(), vals.end(), x) - vals.begin());
      ver.push_back(ver.back().mul(rank, CountSumM::T{1, x}));
    }
  }
  pair<int, i64> count_sum(int l, int r, int x) const {
    const int xl = int(upper_bound(vals.begin(), vals.end(), x) - vals.begin());
    auto sub = [](CountSumM::T b, CountSumM::T a) { return CountSumM::T{b.c - a.c, b.s - a.s}; };
    const auto t = PST::prod_diff(ver[l], ver[r], 0, xl, sub);
    return {t.c, t.s};
  }
};
