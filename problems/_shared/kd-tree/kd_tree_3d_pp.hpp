#pragma once
// 3 次元 KD 木で、直方体の範囲のモノイドの値を返す。1 点につき 1 ノードを置く形 (点を節に置く)。
// 深さの 3 で割った余りで x、y、z の順に軸を選び、中央値で分ける。各節は外接直方体 (半開) と
// 部分木の値と自分の点の値を持つ。範囲 [l, r) x [d, u) x [e, f) を取る。
#include <algorithm>
#include <numeric>
#include <vector>

template <class M, class T = typename M::T> class KdTree3DPP {
 public:
  KdTree3DPP() {}
  KdTree3DPP(const std::vector<int> &x, const std::vector<int> &y, const std::vector<int> &z,
             const std::vector<T> &v) {
    n_ = (int)x.size();
    px_ = x;
    py_ = y;
    pz_ = z;
    pv_ = v;
    t_.reserve(n_);
    if (n_ == 0) return;
    std::vector<int> idx(n_);
    std::iota(idx.begin(), idx.end(), 0);
    root_ = build(idx, 0, n_, 0);
  }

  int size() const { return n_; }

  T prod(int l, int d, int e, int r, int u, int f) const {
    if (l >= r || d >= u || e >= f || n_ == 0) return M::ti();
    return prod_(root_, l, d, e, r, u, f);
  }

 private:
  struct Node {
    int lx, ly, lz, hx, hy, hz;  // 外接直方体 (半開)
    int px, py, pz;               // 自分の点の座標
    int ch[2];                    // 子。無ければ -1
    T val;                        // 自分の点の値
    T sum;                        // 部分木の値
  };

  int n_ = 0, root_ = -1;
  std::vector<Node> t_;
  std::vector<int> px_, py_, pz_;
  std::vector<T> pv_;

  int build(std::vector<int> &idx, int l, int r, int dep) {
    if (l >= r) return -1;
    const int mid = (l + r) >> 1;
    const std::vector<int> &cx = px_, &cy = py_, &cz = pz_;
    const int dim = dep % 3;
    if (dim == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cx](int a, int b) { return cx[a] < cx[b]; });
    else if (dim == 1)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cy](int a, int b) { return cy[a] < cy[b]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cz](int a, int b) { return cz[a] < cz[b]; });
    const int v = (int)t_.size();
    t_.emplace_back();
    const int i = idx[mid];
    const int ia = build(idx, l, mid, dep + 1);
    const int ib = build(idx, mid + 1, r, dep + 1);
    Node &p = t_[v];
    p.px = px_[i];
    p.py = py_[i];
    p.pz = pz_[i];
    p.ch[0] = ia;
    p.ch[1] = ib;
    p.val = pv_[i];
    p.lx = p.px;
    p.ly = p.py;
    p.lz = p.pz;
    p.hx = p.px + 1;
    p.hy = p.py + 1;
    p.hz = p.pz + 1;
    p.sum = p.val;
    if (ia >= 0) {
      const Node &c = t_[ia];
      p.lx = std::min(p.lx, c.lx);
      p.ly = std::min(p.ly, c.ly);
      p.lz = std::min(p.lz, c.lz);
      p.hx = std::max(p.hx, c.hx);
      p.hy = std::max(p.hy, c.hy);
      p.hz = std::max(p.hz, c.hz);
      p.sum = M::op(p.sum, c.sum);
    }
    if (ib >= 0) {
      const Node &c = t_[ib];
      p.lx = std::min(p.lx, c.lx);
      p.ly = std::min(p.ly, c.ly);
      p.lz = std::min(p.lz, c.lz);
      p.hx = std::max(p.hx, c.hx);
      p.hy = std::max(p.hy, c.hy);
      p.hz = std::max(p.hz, c.hz);
      p.sum = M::op(p.sum, c.sum);
    }
    return v;
  }

  T prod_(int v, int l, int d, int e, int r, int u, int f) const {
    const Node &p = t_[v];
    if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly || p.hz <= e || f <= p.lz) return M::ti();
    if (l <= p.lx && p.hx <= r && d <= p.ly && p.hy <= u && e <= p.lz && p.hz <= f) return p.sum;
    T ret = M::ti();
    if (l <= p.px && p.px < r && d <= p.py && p.py < u && e <= p.pz && p.pz < f) ret = p.val;
    if (p.ch[0] >= 0) ret = M::op(ret, prod_(p.ch[0], l, d, e, r, u, f));
    if (p.ch[1] >= 0) ret = M::op(ret, prod_(p.ch[1], l, d, e, r, u, f));
    return ret;
  }
};
