#pragma once
// 四隅の差分配列を 2 次元 BIT に載せる (reference.hpp と同じ方式) の、配列を 1 本に詰めた形。
//
// 長方形 [l, r) x [d, u) への w の加算は、差分配列の四隅 (l, d) += w、(r, d) -= w、
// (l, u) -= w、(r, u) += w の点加算に直り、点 (x, y) の値は x 以下 y 以下の四隅の和になる。
// x を座標圧縮し、x の各節が持つ y を 1 本の配列に連結し、その上に節ごとの BIT を
// 同じく連結して置く。x の圧縮の添字は、値の範囲を 65536 個に分けた表と短い線形走査で引く。
#include <algorithm>
#include <array>
#include <vector>
#include "pj.hpp"

namespace bit2d_flat_internal {
// 座標の値から添字を引く表。値の範囲を 65536 個以下の区間に分け、区間の先頭の添字から
// 線形に進める。
struct Lookup {
  std::vector<int> v;  // 昇順、重複なし
  unsigned lo = 0, sh = 0;
  std::vector<int> t;  // t[k] は (v - lo) >> sh が k 未満のものの数
  void build() {
    t.clear();
    if (v.empty()) return;
    lo = (unsigned)v[0];
    const unsigned d = (unsigned)(v.back() - v[0]);
    sh = 0;
    while ((d >> sh) >= (1u << 16)) ++sh;
    const int K = (int)(d >> sh) + 1, m = (int)v.size();
    t.resize(K + 1);
    for (int i = 0, k = 0; k <= K; ++k) {
      while (i < m && (int)(((unsigned)(v[i] - (int)lo)) >> sh) < k) ++i;
      t[k] = i;
    }
  }
  // x 以上の最小の添字。
  int lower(int x) const {
    const int m = (int)v.size();
    if (m == 0 || x <= v[0]) return 0;
    if (v.back() < x) return m;
    int a = t[(unsigned)(x - (int)lo) >> sh];
    while (v[a] < x) ++a;
    return a;
  }
  // x 以下の個数。
  int upper(int x) const {
    const int m = (int)v.size();
    if (m == 0 || x < v[0]) return 0;
    if (v.back() <= x) return m;
    int a = t[(unsigned)(x - (int)lo) >> sh];
    while (v[a] <= x) ++a;
    return a;
  }
};
}  // namespace bit2d_flat_internal

struct Solver {
  Solver(const vector<array<i64, 5>> &rect, const vector<array<i64, 6>> &query)
      : rect_(rect), query_(query) {}

  void run();
  const vector<i64> &answers() const { return ans_; }

 private:
  const vector<array<i64, 5>> &rect_;
  const vector<array<i64, 6>> &query_;

  bit2d_flat_internal::Lookup xs_;  // 四隅の x
  vector<int> yv_;                  // 節ごとの y を連結したもの
  vector<int> yoff_, ylen_;         // 節 i の y は yv_[yoff_[i] .. + ylen_[i])
  vector<i64> bit_;                 // 節ごとの BIT を連結したもの
  vector<int> boff_, bloc_;         // 節 i の BIT は bit_[boff_[i] .. + bloc_[i])
  vector<i64> ans_;

  void add_point(i64 x, i64 y, i64 w) {
    const int nx = (int)xs_.v.size();
    for (int i = xs_.lower((int)x) + 1; i <= nx; i += i & -i) {
      const int n = ylen_[i], b = yoff_[i];
      int j = (int)(std::lower_bound(yv_.begin() + b, yv_.begin() + b + n, (int)y) - yv_.begin()) - b + 1;
      i64 *d = bit_.data() + boff_[i];
      for (; j <= n; j += j & -j) d[j] += w;
    }
  }

  i64 prefix_sum(i64 x, i64 y) const {
    i64 s = 0;
    for (int i = xs_.upper((int)x); i > 0; i -= i & -i) {
      const int n = ylen_[i], b = yoff_[i];
      int j = (int)(std::upper_bound(yv_.begin() + b, yv_.begin() + b + n, (int)y) - yv_.begin()) - b;
      const i64 *d = bit_.data() + boff_[i];
      for (; j > 0; j -= j & -j) s += d[j];
    }
    return s;
  }

  void push_corners(i64 l, i64 d, i64 r, i64 u, i64 w) {
    add_point(l, d, w);
    add_point(r, d, -w);
    add_point(l, u, -w);
    add_point(r, u, w);
  }
};

inline void Solver::run() {
  // 四隅を集める。初期の長方形と、あとから足す長方形の両方。
  vector<array<i64, 3>> cs;  // {x, y, w}
  cs.reserve(4 * (rect_.size() + query_.size()));
  auto collect = [&cs](i64 l, i64 d, i64 r, i64 u, i64 w) {
    cs.push_back({l, d, w});
    cs.push_back({r, d, -w});
    cs.push_back({l, u, -w});
    cs.push_back({r, u, w});
  };
  for (const auto &e : rect_) collect(e[0], e[1], e[2], e[3], e[4]);
  for (const auto &e : query_)
    if (e[0] == 0) collect(e[1], e[2], e[3], e[4], e[5]);

  // x を圧縮する。
  xs_.v.clear();
  xs_.v.reserve(cs.size());
  for (const auto &c : cs) xs_.v.push_back((int)c[0]);
  sort(xs_.v.begin(), xs_.v.end());
  xs_.v.erase(unique(xs_.v.begin(), xs_.v.end()), xs_.v.end());
  xs_.build();
  const int nx = (int)xs_.v.size();

  // 四隅の y を、その x が通る x の節すべてに積む。
  vector<int> xi(cs.size());
  vector<int> cnt(nx + 2, 0);
  for (size_t j = 0; j < cs.size(); ++j) {
    xi[j] = xs_.lower((int)cs[j][0]) + 1;
    for (int i = xi[j]; i <= nx; i += i & -i) ++cnt[i];
  }
  yoff_.assign(nx + 2, 0);
  for (int i = 1; i <= nx; ++i) yoff_[i + 1] = yoff_[i] + cnt[i];
  yv_.resize(yoff_[nx + 1]);
  {
    vector<int> fill(yoff_.begin(), yoff_.end());
    for (size_t j = 0; j < cs.size(); ++j)
      for (int i = xi[j]; i <= nx; i += i & -i) yv_[fill[i]++] = (int)cs[j][1];
  }

  // 節ごとに整列して重複を落とし、前に詰め直す。
  ylen_.assign(nx + 2, 0);
  {
    vector<int> start(nx + 2, 0);
    int w = 0;
    for (int i = 1; i <= nx; ++i) {
      const int b = yoff_[i], e = yoff_[i + 1];
      sort(yv_.begin() + b, yv_.begin() + e);
      const int u = (int)(unique(yv_.begin() + b, yv_.begin() + e) - (yv_.begin() + b));
      start[i] = w;
      for (int j = 0; j < u; ++j) yv_[w + j] = yv_[b + j];
      ylen_[i] = u;
      w += u;
    }
    yv_.resize(w);
    yoff_ = std::move(start);
  }

  // 節ごとの BIT の置き場。
  boff_.assign(nx + 2, 0);
  bloc_.assign(nx + 2, 0);
  {
    int w = 0;
    for (int i = 1; i <= nx; ++i) {
      boff_[i] = w;
      bloc_[i] = ylen_[i] + 1;
      w += bloc_[i];
    }
    bit_.assign(w, 0);
  }

  // 時間順に処理する。初期の長方形はクエリより先に全部足しておく。
  for (const auto &e : rect_) push_corners(e[0], e[1], e[2], e[3], e[4]);
  ans_.clear();
  ans_.reserve(query_.size());
  for (const auto &e : query_) {
    if (e[0] == 0) push_corners(e[1], e[2], e[3], e[4], e[5]);
    else ans_.push_back(prefix_sum(e[1], e[2]));
  }
}
