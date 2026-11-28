#pragma once
// 2 次元 KD 木。1 点につき 1 ノードを置く形 (点を節に置き、左右の部分木に残りを分ける)。
// 深さで x と y を交互に、中央値で分ける。各節は外接矩形 (半開)、部分木のモノイドの値、
// 自分の点の値を持つ。モノイド M は M::T、M::op、M::ti を持つ。
// 点の値は set (置き換え) と mul (op で足す) で変えられ、範囲 [l, r) x [d, u) の値を返す。
//
// 1 点の更新は O(log N)、長方形の値は O(sqrt N) (打ち切ると丸ごと加算)。
#include <algorithm>
#include <numeric>
#include <vector>

template <class M, class T = typename M::T> class KdTree2D {
 public:
  KdTree2D() {}
  KdTree2D(const std::vector<int> &x, const std::vector<int> &y, const std::vector<T> &v) {
    n_ = (int)x.size();
    px_ = x;
    py_ = y;
    cur_ = v;
    t_.reserve(n_);
    pos_.assign(n_, -1);
    if (n_ == 0) return;
    std::vector<int> idx(n_);
    std::iota(idx.begin(), idx.end(), 0);
    root_ = build(idx, 0, n_, 0, -1);
  }

  int size() const { return (int)cur_.size(); }

  void set(int i, T v) {
    cur_[i] = v;
    int p = pos_[i];
    t_[p].val = v;
    for (; p >= 0; p = t_[p].par) pull(p);
  }

  void mul(int i, T v) { set(i, M::op(cur_[i], v)); }

  T prod(int l, int d, int r, int u) const {
    if (l >= r || d >= u || n_ == 0) return M::ti();
    return prod_(root_, l, d, r, u);
  }

 private:
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    int px, py;          // 自分の点の座標
    int ch[2];           // 子。無ければ -1
    int par;             // 親。根は -1
    T val;               // 自分の点の値
    T sum;               // 部分木の値
  };

  int n_ = 0, root_ = -1;
  std::vector<Node> t_;
  std::vector<int> px_, py_, pos_;
  std::vector<T> cur_;

  void pull(int v) {
    Node &p = t_[v];
    p.sum = p.val;
    if (p.ch[0] >= 0) p.sum = M::op(p.sum, t_[p.ch[0]].sum);
    if (p.ch[1] >= 0) p.sum = M::op(p.sum, t_[p.ch[1]].sum);
  }

  int build(std::vector<int> &idx, int l, int r, int dep, int par) {
    if (l >= r) return -1;
    const int mid = (l + r) >> 1;
    const std::vector<int> &cx = px_, &cy = py_;
    if ((dep & 1) == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cx](int a, int b) { return cx[a] < cx[b]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cy](int a, int b) { return cy[a] < cy[b]; });
    const int v = (int)t_.size();
    t_.emplace_back();
    const int i = idx[mid];
    const int ia = build(idx, l, mid, dep + 1, v);
    const int ib = build(idx, mid + 1, r, dep + 1, v);
    Node &p = t_[v];
    p.px = px_[i];
    p.py = py_[i];
    p.ch[0] = ia;
    p.ch[1] = ib;
    p.par = par;
    p.val = cur_[i];
    p.lx = p.px;
    p.ly = p.py;
    p.hx = p.px + 1;
    p.hy = p.py + 1;
    if (ia >= 0) {
      const Node &c = t_[ia];
      p.lx = std::min(p.lx, c.lx);
      p.ly = std::min(p.ly, c.ly);
      p.hx = std::max(p.hx, c.hx);
      p.hy = std::max(p.hy, c.hy);
    }
    if (ib >= 0) {
      const Node &c = t_[ib];
      p.lx = std::min(p.lx, c.lx);
      p.ly = std::min(p.ly, c.ly);
      p.hx = std::max(p.hx, c.hx);
      p.hy = std::max(p.hy, c.hy);
    }
    pull(v);
    pos_[i] = v;
    return v;
  }

  T prod_(int v, int l, int d, int r, int u) const {
    const Node &p = t_[v];
    if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly) return M::ti();
    if (l <= p.lx && p.hx <= r && d <= p.ly && p.hy <= u) return p.sum;
    T ret = M::ti();
    if (l <= p.px && p.px < r && d <= p.py && p.py < u) ret = p.val;
    if (p.ch[0] >= 0) ret = M::op(ret, prod_(p.ch[0], l, d, r, u));
    if (p.ch[1] >= 0) ret = M::op(ret, prod_(p.ch[1], l, d, r, u));
    return ret;
  }
};
