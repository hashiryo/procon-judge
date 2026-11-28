#pragma once
// 2 次元 KD 木。点を葉に置く完全 2 分木で、節の添字はセグメント木と同じ (子は 2v と 2v+1)。
// 深さで x と y を交互に、中央値で分ける。各節は外接矩形 (半開) と、部分木のモノイドの値を持つ。
// モノイド M は M::T、M::op、M::ti を持つ。点の値は set (置き換え) と mul (op で足す) で変えられる。
// 範囲は半開 [l, r) x [d, u) で受け、空なら M::ti() を返す。
//
// 外接矩形は構築のあと変わらないので、更新のたびに直すのは和だけ。
// 1 点の更新は O(log N)、長方形の値は O(sqrt N) (打ち切りと丸ごと加算)。
#include <algorithm>
#include <numeric>
#include <vector>

template <class M, class T = typename M::T> class KdTree2D {
 public:
  KdTree2D() {}
  KdTree2D(const std::vector<int> &x, const std::vector<int> &y, const std::vector<T> &v) {
    const int n = (int)x.size();
    xs_ = x;
    ys_ = y;
    val_ = v;
    t_.assign((size_t)4 * n + 4, Node{0, 0, 0, 0, M::ti()});
    pos_.assign(n, 0);
    if (n == 0) return;
    std::vector<int> idx(n);
    std::iota(idx.begin(), idx.end(), 0);
    build(idx, 1, 0, n, 0);
  }

  int size() const { return (int)val_.size(); }

  // 点 i の値を v にする。
  void set(int i, T v) {
    val_[i] = v;
    int p = pos_[i];
    t_[p].sum = v;
    for (p >>= 1; p; p >>= 1) pull(p);
  }

  // 点 i の値を M::op(今の値, v) にする。
  void mul(int i, T v) { set(i, M::op(val_[i], v)); }

  // [l, r) x [d, u) の点の値の M::op。空なら M::ti()。
  T prod(int l, int d, int r, int u) const {
    if (l >= r || d >= u) return M::ti();
    return prod_(1, l, d, r, u);
  }

 private:
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    T sum;
  };

  std::vector<Node> t_;
  std::vector<int> xs_, ys_;
  std::vector<T> val_;
  std::vector<int> pos_;  // 点 -> 葉の節

  void pull(int v) {
    const Node &lc = t_[2 * v], &rc = t_[2 * v + 1];
    t_[v].sum = M::op(lc.sum, rc.sum);
  }

  void build(std::vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      pos_[i] = v;
      t_[v] = Node{xs_[i], ys_[i], xs_[i] + 1, ys_[i] + 1, val_[i]};
      return;
    }
    const int mid = (l + r) >> 1;
    const std::vector<int> &cx = xs_, &cy = ys_;
    if ((dep & 1) == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cx](int p, int q) { return cx[p] < cx[q]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cy](int p, int q) { return cy[p] < cy[q]; });
    build(idx, 2 * v, l, mid, dep + 1);
    build(idx, 2 * v + 1, mid, r, dep + 1);
    Node &p = t_[v];
    const Node &lc = t_[2 * v], &rc = t_[2 * v + 1];
    p.lx = std::min(lc.lx, rc.lx);
    p.ly = std::min(lc.ly, rc.ly);
    p.hx = std::max(lc.hx, rc.hx);
    p.hy = std::max(lc.hy, rc.hy);
    p.sum = M::op(lc.sum, rc.sum);
  }

  T prod_(int v, int l, int d, int r, int u) const {
    const Node &p = t_[v];
    if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly) return M::ti();
    if (l <= p.lx && p.hx <= r && d <= p.ly && p.hy <= u) return p.sum;
    return M::op(prod_(2 * v, l, d, r, u), prod_(2 * v + 1, l, d, r, u));
  }
};
