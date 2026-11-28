#pragma once
// NeoLibrary の永続セグメント木 (neo/base/ds/range/PersistentSegmentTree.hpp) で range-kth-smallest を解く。
// 値の順位を葉に持ち、版 i を先頭 i 個を入れた木にする。版 r と版 l を同時に下りて k 番目を探す。
// 静的な問い合わせは wavelet matrix の方が速い (procon-judge の記録)。ここは差を見るための比較。
#include <algorithm>
#include <cstddef>
#include "pj.hpp"
#include "neo/base/ds/range/PersistentSegmentTree.hpp"
#include "neo/base/ds/range/segtree/Sum.hpp"

struct Solver {
  using PST = PersistentSegmentTree<monoid::Sum<int>>;
  vector<int> vals;
  vector<PST> ver;
  explicit Solver(const vector<int> &a) : vals(a) {
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
    // 1 回の mul で増えるのは根から葉までの道 (log2(sz) + 1 節) だけ。
    PST::reserve(std::size_t(a.size()) * 20 + 1);
    ver.reserve(a.size() + 1);
    ver.push_back(PST(int(vals.size())));
    for (int x : a) {
      const int rank = int(lower_bound(vals.begin(), vals.end(), x) - vals.begin());
      ver.push_back(ver.back().mul(rank, 1));
    }
  }
  int kth_smallest(int l, int r, int k) const { return vals[PST::kth(ver[l], ver[r], k)]; }
};
