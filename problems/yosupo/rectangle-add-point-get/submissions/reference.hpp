#pragma once
#include <algorithm>
#include "pj.hpp"

// 2 次元の差分配列 + 2 次元 BIT (x の BIT の各節が y の BIT)。
//
// 長方形 [l, r) x [d, u) への w の加算は、差分配列の四隅
//   (l, d) += w, (r, d) -= w, (l, u) -= w, (r, u) += w
// への点加算に直る。点 (x, y) の値は、x 以下 y 以下の四隅の和、つまり 2 次元の
// prefix sum になる。四隅の座標は先に全部読めるので、x と y をそれぞれ座標圧縮して
// 2 次元 BIT に載せる。1 つの加算もクエリも O(log^2 (N + Q))。
//
// 初期の長方形とクエリの長方形の四隅は高々 4 (N + Q) 個、重みの和は高々
// (N + Q) * 10^9 なので 64 bit に収まる。
//
// 圧縮と木の構築も run() の中でやる。構築だけを計測区間の外に出すと、この問題では
// 測るものがクエリの処理だけになってしまうため。
struct Solver {
  Solver(const vector<array<i64, 5>> &rect,   // 初期の長方形 {l, d, r, u, w}
         const vector<array<i64, 6>> &query)  // {0, l, d, r, u, w} か {1, x, y, 0, 0, 0}
      : rect_(rect), query_(query) {}

  void run();
  const vector<i64> &answers() const { return ans_; }

 private:
  const vector<array<i64, 5>> &rect_;
  const vector<array<i64, 6>> &query_;

  vector<i64> xs_;           // 四隅の x (昇順、重複なし)
  vector<vector<i64>> ys_;   // x の各節が持つ四隅の y (昇順、重複なし)
  vector<vector<i64>> bit_;  // ys_ と同じ形の BIT
  vector<i64> ans_;

  // 差分配列への点加算。x と y は四隅として集めた座標。
  void add_point(i64 x, i64 y, i64 w) {
    const int nx = (int)xs_.size();
    int i = int(lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin()) + 1;
    for (int j = i; j <= nx; j += j & -j) {
      const auto &vy = ys_[j];
      auto &bt = bit_[j];
      int p = int(lower_bound(vy.begin(), vy.end(), y) - vy.begin()) + 1;
      for (int k = p; k < (int)bt.size(); k += k & -k) bt[k] += w;
    }
  }

  // x 以下 y 以下の四隅の和。
  i64 prefix_sum(i64 x, i64 y) const {
    i64 s = 0;
    int i = int(upper_bound(xs_.begin(), xs_.end(), x) - xs_.begin());
    for (int j = i; j > 0; j -= j & -j) {
      const auto &vy = ys_[j];
      const auto &bt = bit_[j];
      int p = int(upper_bound(vy.begin(), vy.end(), y) - vy.begin());
      for (int k = p; k > 0; k -= k & -k) s += bt[k];
    }
    return s;
  }
};

inline void Solver::run() {
  // 四隅 {x, y, w} を集める。初期の長方形と、あとから足す長方形の両方。
  vector<array<i64, 3>> corners;
  corners.reserve(4 * (rect_.size() + query_.size()));
  auto push_corners = [&corners](i64 l, i64 d, i64 r, i64 u, i64 w) {
    corners.push_back({l, d, w});
    corners.push_back({r, d, -w});
    corners.push_back({l, u, -w});
    corners.push_back({r, u, w});
  };
  for (const auto &e : rect_) push_corners(e[0], e[1], e[2], e[3], e[4]);
  for (const auto &e : query_)
    if (e[0] == 0) push_corners(e[1], e[2], e[3], e[4], e[5]);

  // x を圧縮する。
  xs_.reserve(corners.size());
  for (const auto &c : corners) xs_.push_back(c[0]);
  sort(xs_.begin(), xs_.end());
  xs_.erase(unique(xs_.begin(), xs_.end()), xs_.end());
  const int nx = (int)xs_.size();

  // 四隅の y を、その x が通る x の節すべてに積んでから、節ごとに整列して重複を落とす。
  ys_.assign(nx + 1, {});
  for (const auto &c : corners) {
    int i = int(lower_bound(xs_.begin(), xs_.end(), c[0]) - xs_.begin()) + 1;
    for (int j = i; j <= nx; j += j & -j) ys_[j].push_back(c[1]);
  }
  bit_.assign(nx + 1, {});
  for (int j = 1; j <= nx; ++j) {
    sort(ys_[j].begin(), ys_[j].end());
    ys_[j].erase(unique(ys_[j].begin(), ys_[j].end()), ys_[j].end());
    bit_[j].assign(ys_[j].size() + 1, 0);
  }

  // 時間順に処理する。初期の長方形はクエリより先に全部足しておく。
  for (const auto &e : rect_) {
    i64 w = e[4];
    add_point(e[0], e[1], w);
    add_point(e[2], e[1], -w);
    add_point(e[0], e[3], -w);
    add_point(e[2], e[3], w);
  }
  ans_.clear();
  ans_.reserve(query_.size());
  for (const auto &e : query_) {
    if (e[0] == 0) {
      i64 w = e[5];
      add_point(e[1], e[2], w);
      add_point(e[3], e[2], -w);
      add_point(e[1], e[4], -w);
      add_point(e[3], e[4], w);
    } else {
      ans_.push_back(prefix_sum(e[1], e[2]));
    }
  }
}
