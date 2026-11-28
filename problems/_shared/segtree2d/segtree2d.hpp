#pragma once
// 2 次元の点の集合に対するモノイドの範囲積 (merge sort 木)。x を座標圧縮してセグメント木の
// 葉に載せ、各節はその x の範囲に入る点の y を昇順に持つ。節ごとに、y の並びの上に
// モノイドのセグメント木 (2 の冪に切り上げた配列) を置く。
//
// 範囲 [l, r) x [d, u) の値は O(log^2 N)、1 点の更新も O(log^2 N)、構築は O(N log N)。
// モノイド M は M::T、M::op、M::ti を持ち、op は可換とする (和、min、max、xor など)。
// 同じ (x, y) の点が複数あると、値は op でまとまる。更新は (x, y) の値を置き換える。
#include <algorithm>
#include <vector>

template <class M, class T = typename M::T> class SegTree2D {
 public:
  SegTree2D() {}
  // 点 (x[i], y[i]) に値 v[i] を載せる。
  SegTree2D(const std::vector<int> &x, const std::vector<int> &y, const std::vector<T> &v) {
    const int n = (int)x.size();
    px_ = x;
    py_ = y;
    cur_ = v;
    xs_ = x;
    std::sort(xs_.begin(), xs_.end());
    xs_.erase(std::unique(xs_.begin(), xs_.end()), xs_.end());
    nx_ = (int)xs_.size();
    sz_ = 1;
    while (sz_ < nx_) sz_ <<= 1;
    nodes_ = 2 * sz_;
    yoff_.assign(nodes_, 0);
    ylen_.assign(nodes_, 0);
    // 各節の y の並びの長さを数えて置き場を決める。
    std::vector<int> cnt(nodes_, 0);
    std::vector<int> xi(n);
    for (int i = 0; i < n; ++i) {
      xi[i] = (int)(std::lower_bound(xs_.begin(), xs_.end(), x[i]) - xs_.begin());
      for (int p = xi[i] + sz_; p; p >>= 1) ++cnt[p];
    }
    int total = 0;
    for (int p = 1; p < nodes_; ++p) {
      yoff_[p] = total;
      total += cnt[p];
    }
    yv_.resize(total);
    {
      std::vector<int> fill(yoff_.begin(), yoff_.begin() + nodes_);
      for (int i = 0; i < n; ++i)
        for (int p = xi[i] + sz_; p; p >>= 1) yv_[fill[p]++] = y[i];
    }
    // 節ごとに y を整列して重複を落とし、前に詰め直す。
    {
      int w = 0;
      for (int p = 1; p < nodes_; ++p) {
        const int b = yoff_[p], e = b + cnt[p];
        std::sort(yv_.begin() + b, yv_.begin() + e);
        const int u = (int)(std::unique(yv_.begin() + b, yv_.begin() + e) - (yv_.begin() + b));
        yoff_[p] = w;
        for (int j = 0; j < u; ++j) yv_[w + j] = yv_[b + j];
        ylen_[p] = u;
        w += u;
      }
      yv_.resize(w);
    }
    // 節ごとのセグメント木の置き場 (y の数以上で最小の 2 の冪)。
    voff_.assign(nodes_, 0);
    vsz_.assign(nodes_, 0);
    {
      int tot = 0;
      for (int p = 1; p < nodes_; ++p) {
        int s = 1;
        while (s < ylen_[p]) s <<= 1;
        voff_[p] = tot;
        vsz_[p] = s;
        tot += 2 * s;
      }
      val_.assign(tot, M::ti());
    }
    // 葉 (x の葉) に点の値を置く。同じ (x, y) は op でまとまる。
    for (int i = 0; i < n; ++i) {
      const int p = xi[i] + sz_;
      const int j = yindex(p, y[i]);
      T &cell = val_[voff_[p] + vsz_[p] + j];
      cell = M::op(cell, v[i]);
    }
    for (int p = sz_; p < nodes_; ++p) build_tree(p);
    // 内部の節は、子の y の並びを突き合わせて値をまとめる。
    for (int p = sz_ - 1; p >= 1; --p) {
      const int lc = 2 * p, rc = 2 * p + 1;
      T *d = val_.data() + voff_[p];
      for (int j = 0; j < ylen_[p]; ++j) {
        const int y = yv_[yoff_[p] + j];
        d[vsz_[p] + j] = M::op(child_value(lc, y), child_value(rc, y));
      }
      build_tree(p);
    }
  }

  int size() const { return (int)cur_.size(); }

  // 点 i の (x, y) の値を v にする。
  void set(int i, T v) {
    cur_[i] = v;
    update(i);
  }
  // 点 i の (x, y) の値を M::op(今の値, v) にする。
  void mul(int i, T v) {
    cur_[i] = M::op(cur_[i], v);
    update(i);
  }

  // [l, r) x [d, u) の値。空なら M::ti()。
  T prod(int l, int d, int r, int u) const {
    if (l >= r || d >= u) return M::ti();
    const int xl = (int)(std::lower_bound(xs_.begin(), xs_.end(), l) - xs_.begin());
    const int xr = (int)(std::lower_bound(xs_.begin(), xs_.end(), r) - xs_.begin());
    T ret = M::ti();
    int a = xl + sz_, b = xr + sz_;
    while (a < b) {
      if (a & 1) ret = M::op(ret, node_prod(a++, d, u));
      if (b & 1) ret = M::op(ret, node_prod(--b, d, u));
      a >>= 1;
      b >>= 1;
    }
    return ret;
  }

 private:
  int nx_ = 0, sz_ = 0, nodes_ = 0;
  std::vector<int> xs_, px_, py_;
  std::vector<int> yoff_, ylen_;   // 節の y の並び
  std::vector<int> voff_, vsz_;    // 節のセグメント木の置き場と大きさ
  std::vector<int> yv_;
  std::vector<T> val_, cur_;

  int yindex(int p, int y) const {
    const int *b = yv_.data() + yoff_[p];
    return (int)(std::lower_bound(b, b + ylen_[p], y) - b);
  }

  // 節 p の y = y の値。無ければ M::ti()。
  T child_value(int p, int y) const {
    if (ylen_[p] == 0) return M::ti();
    const int j = yindex(p, y);
    if (j >= ylen_[p] || yv_[yoff_[p] + j] != y) return M::ti();
    return val_[voff_[p] + vsz_[p] + j];
  }

  // 節 p のセグメント木を、葉から上へ作る (構築用)。
  void build_tree(int p) {
    T *d = val_.data() + voff_[p];
    for (int j = vsz_[p] - 1; j >= 1; --j) d[j] = M::op(d[2 * j], d[2 * j + 1]);
  }

  // 節 p の y の並びの j 番目を v にして、その節の木の根まで直す。
  void set_leaf(int p, int j, T v) {
    T *d = val_.data() + voff_[p];
    int k = vsz_[p] + j;
    d[k] = v;
    for (k >>= 1; k; k >>= 1) d[k] = M::op(d[2 * k], d[2 * k + 1]);
  }

  T node_prod(int p, int d, int u) const {
    const int n = ylen_[p], base = yoff_[p];
    const int *b = yv_.data() + base;
    int lo = (int)(std::lower_bound(b, b + n, d) - b);
    int hi = (int)(std::lower_bound(b, b + n, u) - b);
    if (lo >= hi) return M::ti();
    const T *t = val_.data() + voff_[p];
    T ret = M::ti();
    for (lo += vsz_[p], hi += vsz_[p]; lo < hi; lo >>= 1, hi >>= 1) {
      if (lo & 1) ret = M::op(ret, t[lo++]);
      if (hi & 1) ret = M::op(ret, t[--hi]);
    }
    return ret;
  }

  void update(int i) {
    const int x = px_[i], y = py_[i];
    int p = (int)(std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin()) + sz_;
    // 葉は点の値をそのまま置き、上は子から直す。直すのは経路の節の木の 1 本だけ。
    set_leaf(p, yindex(p, y), cur_[i]);
    for (p >>= 1; p; p >>= 1)
      set_leaf(p, yindex(p, y), M::op(child_value(2 * p, y), child_value(2 * p + 1, y)));
  }
};
