#pragma once
// 2 次元 KD 木で、点の集合に長方形の作用を積み、点の値を読む (双対の形)。今の Library の KDTree と
// 同じく 1 点につき 1 ノードを置く形 (点を節に置き、左右の部分木に残りを分ける)。
// 各節は外接矩形 (半開)、作用 (a, b)、自分の点の値を持つ。作用は値に v -> a v + b を当てる。
// 長方形が節の外接矩形を丸ごと覆えば節に積み (自分の点の値にも当てる)、部分的なら子に配ってから
// 自分の点を見て降りる。点の値は、根からその節まで配りながら読む。
// 環は Ring (W、mul、add、one、zero) で受ける。
#include <algorithm>
#include <numeric>
#include <vector>

template <class Ring> class KdDualPP {
 public:
  using W = typename Ring::W;

  KdDualPP() {}
  KdDualPP(const std::vector<int> &x, const std::vector<int> &y, const std::vector<W> &init) {
    n_ = (int)x.size();
    px_ = x;
    py_ = y;
    init_ = init;
    t_.reserve(n_);
    pos_.assign(n_, -1);
    if (n_ == 0) return;
    std::vector<int> idx(n_);
    std::iota(idx.begin(), idx.end(), 0);
    root_ = build(idx, 0, n_, 0, -1);
  }

  int size() const { return n_; }

  // [l, r) x [d, u) の点に v -> a v + b を当てる。
  void apply(int l, int d, int r, int u, W a, W b) {
    if (l >= r || d >= u || n_ == 0) return;
    apply_(root_, l, d, r, u, a, b);
  }

  // 点 i の値。根からその節まで作用を配ってから読む。
  W get(int i) {
    int path[64];
    int len = 0;
    for (int v = pos_[i]; v >= 0; v = t_[v].par) path[len++] = v;
    for (int j = len - 1; j >= 1; --j) push(path[j]);
    return t_[pos_[i]].own;
  }

 private:
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    int px, py;          // 自分の点の座標
    int ch[2];           // 子。無ければ -1
    int par;             // 親。根は -1
    W a, b;              // 作用。恒等は (1, 0)
    W own;               // 自分の点の値
  };

  int n_ = 0, root_ = -1;
  std::vector<Node> t_;
  std::vector<int> px_, py_, pos_;
  std::vector<W> init_;

  void apply_node(int v, W a, W b) {
    Node &p = t_[v];
    p.own = Ring::add(Ring::mul(a, p.own), b);
    p.a = Ring::mul(a, p.a);
    p.b = Ring::add(Ring::mul(a, p.b), b);
  }

  void push(int v) {
    Node &p = t_[v];
    if (p.a == Ring::one() && p.b == Ring::zero()) return;
    if (p.ch[0] >= 0) apply_node(p.ch[0], p.a, p.b);
    if (p.ch[1] >= 0) apply_node(p.ch[1], p.a, p.b);
    p.a = Ring::one();
    p.b = Ring::zero();
  }

  void apply_(int v, int l, int d, int r, int u, W a, W b) {
    const Node &p = t_[v];
    if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly) return;
    if (l <= p.lx && p.hx <= r && d <= p.ly && p.hy <= u) {
      apply_node(v, a, b);
      return;
    }
    push(v);
    // 自分の点が範囲に入っていれば、その値に当てる。
    if (l <= p.px && p.px < r && d <= p.py && p.py < u) t_[v].own = Ring::add(Ring::mul(a, t_[v].own), b);
    if (p.ch[0] >= 0) apply_(p.ch[0], l, d, r, u, a, b);
    if (p.ch[1] >= 0) apply_(p.ch[1], l, d, r, u, a, b);
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
    p.a = Ring::one();
    p.b = Ring::zero();
    p.own = init_[i];
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
    pos_[i] = v;
    return v;
  }
};

// 64 bit の整数環。作用は v -> a v + b。
struct KdRingI64 {
  using W = long long;
  static W mul(W a, W b) { return a * b; }
  static W add(W a, W b) { return a + b; }
  static W one() { return 1; }
  static W zero() { return 0; }
};

// mod 998244353 の環。
struct KdRingMod {
  using W = unsigned;
  static constexpr W MOD = 998244353;
  static W mul(W a, W b) { return (W)((unsigned long long)a * b % MOD); }
  static W add(W a, W b) {
    const W s = a + b;
    return s >= MOD ? s - MOD : s;
  }
  static W one() { return 1; }
  static W zero() { return 0; }
};
