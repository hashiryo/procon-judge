#pragma once
// 3 次元 KD 木。点を葉に置く完全 2 分木 (子は 2v と 2v+1) で、深さの 3 で割った余りで
// x、y、z の順に軸を選び、中央値で分ける。各節は外接直方体 (半開) と部分木のモノイドの値を持つ。
// モノイド M は M::T、M::op、M::ti を持つ。範囲 [l, r) x [d, u) x [e, f) の値を返す。
#include <algorithm>
#include <numeric>
#include <vector>

template <class M, class T = typename M::T> class KdTree3D {
 public:
  KdTree3D() {}
  KdTree3D(const std::vector<int> &x, const std::vector<int> &y, const std::vector<int> &z,
           const std::vector<T> &v) {
    const int n = (int)x.size();
    xs_ = x;
    ys_ = y;
    zs_ = z;
    val_ = v;
    t_.assign((size_t)4 * n + 4, Node{0, 0, 0, 0, 0, 0, M::ti()});
    if (n == 0) return;
    std::vector<int> idx(n);
    std::iota(idx.begin(), idx.end(), 0);
    build(idx, 1, 0, n, 0);
  }

  int size() const { return (int)val_.size(); }

  // [l, r) x [d, u) x [e, f) の値。空なら M::ti()。
  T prod(int l, int d, int e, int r, int u, int f) const {
    if (l >= r || d >= u || e >= f) return M::ti();
    return prod_(1, l, d, e, r, u, f);
  }

 private:
  struct Node {
    int lx, ly, lz, hx, hy, hz;  // 外接直方体 (半開)
    T sum;
  };

  std::vector<Node> t_;
  std::vector<int> xs_, ys_, zs_;
  std::vector<T> val_;

  void build(std::vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      t_[v] = Node{xs_[i], ys_[i], zs_[i], xs_[i] + 1, ys_[i] + 1, zs_[i] + 1, val_[i]};
      return;
    }
    const int mid = (l + r) >> 1;
    const std::vector<int> &cx = xs_, &cy = ys_, &cz = zs_;
    const int dim = dep % 3;
    if (dim == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cx](int p, int q) { return cx[p] < cx[q]; });
    else if (dim == 1)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cy](int p, int q) { return cy[p] < cy[q]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cz](int p, int q) { return cz[p] < cz[q]; });
    build(idx, 2 * v, l, mid, dep + 1);
    build(idx, 2 * v + 1, mid, r, dep + 1);
    Node &p = t_[v];
    const Node &lc = t_[2 * v], &rc = t_[2 * v + 1];
    p.lx = std::min(lc.lx, rc.lx);
    p.ly = std::min(lc.ly, rc.ly);
    p.lz = std::min(lc.lz, rc.lz);
    p.hx = std::max(lc.hx, rc.hx);
    p.hy = std::max(lc.hy, rc.hy);
    p.hz = std::max(lc.hz, rc.hz);
    p.sum = M::op(lc.sum, rc.sum);
  }

  T prod_(int v, int l, int d, int e, int r, int u, int f) const {
    const Node &p = t_[v];
    if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly || p.hz <= e || f <= p.lz) return M::ti();
    if (l <= p.lx && p.hx <= r && d <= p.ly && p.hy <= u && e <= p.lz && p.hz <= f) return p.sum;
    return M::op(prod_(2 * v, l, d, e, r, u, f), prod_(2 * v + 1, l, d, e, r, u, f));
  }
};
