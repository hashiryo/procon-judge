#pragma once
// 2 次元の点の集合に対するモノイドの範囲積 (merge sort 木)。x を座標圧縮してセグメント木の
// 葉に載せ、各節はその x の範囲に入る点を (y, 点の番号) の順に並べて持つ (重複もそのまま)。
// 節ごとに、その並びの上にモノイドのセグメント木 (葉を点の数にして 2 倍を持つ) を置く。
// 問いを降りるときは、親の並びの位置を子の位置に写す表 tol (先頭から何個が左の子由来か) を
// 使うので、節ごとの二分探索が要らない。二分探索は根の 2 回だけ。
//
// 範囲 [l, r) x [d, u) の値は O(log^2 N)、1 点の更新も O(log^2 N)、構築は O(N log N)。
// モノイド M は M::T、M::op、M::ti を持ち、op は可換とする。
#include <algorithm>
#include <vector>

template <class M, class T = typename M::T> class SegTree2D {
 public:
  SegTree2D() {}
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
    // 各節の並びの長さ (その x の範囲の点の数) と置き場。
    std::vector<int> cnt(nodes_, 0);
    std::vector<int> xi(n);
    for (int i = 0; i < n; ++i) {
      xi[i] = (int)(std::lower_bound(xs_.begin(), xs_.end(), x[i]) - xs_.begin());
      for (int p = xi[i] + sz_; p; p >>= 1) ++cnt[p];
    }
    yoff_.assign(nodes_, 0);
    ylen_.assign(nodes_, 0);
    {
      int total = 0;
      for (int p = 1; p < nodes_; ++p) {
        yoff_[p] = total;
        ylen_[p] = cnt[p];
        total += cnt[p];
      }
      yv_.assign(total, 0);
    }
    // 葉に点を置いて整列する。
    {
      std::vector<int> fill(yoff_.begin(), yoff_.begin() + nodes_);
      for (int i = 0; i < n; ++i) yv_[fill[xi[i] + sz_]++] = pack(y[i], i);
    }
    for (int p = sz_; p < nodes_; ++p) std::sort(yv_.begin() + yoff_[p], yv_.begin() + yoff_[p] + ylen_[p]);
    // tol の置き場 (節ごとに 点の数 + 1)。
    toff_.assign(nodes_, 0);
    {
      int tot = 0;
      for (int p = 1; p < nodes_; ++p) {
        toff_[p] = tot;
        tot += ylen_[p] + 1;
      }
      tol_.assign(tot, 0);
    }
    // 併合のついでに、点ごとの各段の位置 (ypos_) も埋める。
    levels_ = 0;
    for (int q = sz_; q; q >>= 1) ++levels_;
    ypos_.assign((size_t)n * levels_, 0);
    std::vector<int> dep(nodes_, 0);
    {
      dep[1] = levels_ - 1;
      for (int p = 1; p < sz_; ++p) dep[2 * p] = dep[p] - 1, dep[2 * p + 1] = dep[p] - 1;
    }
    // 葉 (段 0) の位置。
    for (int p = sz_; p < nodes_; ++p)
      for (int j = 0; j < ylen_[p]; ++j) ypos_[(size_t)(yv_[yoff_[p] + j] & 0x1fffff) * levels_] = j;
    // 内部の節は子を併合して作る。(y, 番号) の順を保つ。
    for (int p = sz_ - 1; p >= 1; --p) {
      const int lc = 2 * p, rc = 2 * p + 1;
      const long long *lv = yv_.data() + yoff_[lc], *rv = yv_.data() + yoff_[rc];
      const int ln = ylen_[lc], rn = ylen_[rc];
      long long *dv = yv_.data() + yoff_[p];
      int *t = tol_.data() + toff_[p];
      int li = 0, ri = 0;
      int *yp = ypos_.data() + dep[p];
      for (int j = 0; j < ylen_[p]; ++j) {
        if (ri >= rn || (li < ln && lv[li] <= rv[ri]))
          dv[j] = lv[li++];
        else
          dv[j] = rv[ri++];
        t[j + 1] = li;
        yp[(size_t)(dv[j] & 0x1fffff) * levels_] = j;
      }
    }
    // 値の置き場 (葉を点の数にして 2 倍)。
    voff_.assign(nodes_, 0);
    {
      int tot = 0;
      for (int p = 1; p < nodes_; ++p) {
        voff_[p] = tot;
        tot += 2 * ylen_[p];
      }
      val_.assign(tot, M::ti());
    }
    // 値を置く。点 i は、通る節すべての並びに 1 つずつ入る。
    for (int i = 0; i < n; ++i) {
      int lvl = 0;
      for (int p = xi[i] + sz_; p; p >>= 1, ++lvl) val_[voff_[p] + ylen_[p] + ypos_[(size_t)i * levels_ + lvl]] = cur_[i];
    }
    for (int p = 1; p < nodes_; ++p) build_tree(p);
  }

  int size() const { return (int)cur_.size(); }

  void set(int i, T v) {
    cur_[i] = v;
    update(i);
  }
  void mul(int i, T v) {
    cur_[i] = M::op(cur_[i], v);
    update(i);
  }

  T prod(int l, int d, int r, int u) const {
    if (l >= r || d >= u) return M::ti();
    xl_ = (int)(std::lower_bound(xs_.begin(), xs_.end(), l) - xs_.begin());
    xr_ = (int)(std::lower_bound(xs_.begin(), xs_.end(), r) - xs_.begin());
    if (xl_ >= xr_) return M::ti();
    const long long *root = yv_.data() + yoff_[1];
    const int c = (int)(std::lower_bound(root, root + ylen_[1], pack(d, 0)) - root);
    const int e = (int)(std::lower_bound(root, root + ylen_[1], pack(u, 0)) - root);
    return prod_rec(1, 0, sz_, c, e);
  }

 private:
  static long long pack(int y, int id) { return ((long long)y << 21) | (id & 0x1fffff); }

  int nx_ = 0, sz_ = 0, nodes_ = 0;
  std::vector<int> xs_, px_, py_;
  std::vector<int> yoff_, ylen_;
  std::vector<long long> yv_;
  std::vector<int> toff_, tol_;
  std::vector<int> voff_;
  int levels_ = 0;
  std::vector<int> ypos_;
  std::vector<T> val_, cur_;
  mutable int xl_ = 0, xr_ = 0;

  void build_tree(int p) {
    T *d = val_.data() + voff_[p];
    for (int j = ylen_[p] - 1; j >= 1; --j) d[j] = M::op(d[2 * j], d[2 * j + 1]);
  }

  void set_leaf(int p, int j, T v) {
    T *d = val_.data() + voff_[p];
    int k = ylen_[p] + j;
    d[k] = v;
    for (k >>= 1; k; k >>= 1) d[k] = M::op(d[2 * k], d[2 * k + 1]);
  }

  T node_prod_range(int p, int c, int e) const {
    const int n = ylen_[p];
    const T *t = val_.data() + voff_[p];
    T ret = M::ti();
    for (c += n, e += n; c < e; c >>= 1, e >>= 1) {
      if (c & 1) ret = M::op(ret, t[c++]);
      if (e & 1) ret = M::op(ret, t[--e]);
    }
    return ret;
  }

  T prod_rec(int p, int a, int b, int c, int e) const {
    if (b <= xl_ || xr_ <= a || c >= e) return M::ti();
    if (xl_ <= a && b <= xr_) return node_prod_range(p, c, e);
    const int *t = tol_.data() + toff_[p];
    const int m = (a + b) >> 1;
    const int ac = t[c], ae = t[e];
    return M::op(prod_rec(2 * p, a, m, ac, ae), prod_rec(2 * p + 1, m, b, c - ac, e - ae));
  }

  void update(int i) {
    int p = (int)(std::lower_bound(xs_.begin(), xs_.end(), px_[i]) - xs_.begin()) + sz_;
    const int *yp = ypos_.data() + (size_t)i * levels_;
    int lvl = 0;
    for (; p; p >>= 1, ++lvl) set_leaf(p, yp[lvl], cur_[i]);
  }
};
