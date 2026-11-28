#pragma once
// 2 次元 KD 木で、点の集合に長方形の作用を積み、点の値を読む (双対の形)。
// 点を葉に置く完全 2 分木 (子は 2v と 2v+1) で、深さで x と y を交互に、中央値で分ける。
// 各節は外接矩形 (半開) と作用 (a, b) を持ち、作用は値に v -> a v + b を当てる。
// 長方形が節の外接矩形を丸ごと覆えば節に積み、部分的なら子に配ってから降りる。
// 点の値は、根から葉へ作用を配ってから葉の作用を当てて読む。
// 環は Ring (W、mul、add、one、zero) で受ける。
#include <algorithm>
#include <numeric>
#include <vector>

template <class Ring> class KdDual2D {
 public:
  using W = typename Ring::W;

  KdDual2D() {}
  KdDual2D(const std::vector<int> &x, const std::vector<int> &y, const std::vector<W> &init) {
    n_ = (int)x.size();
    xs_ = x;
    ys_ = y;
    init_ = init;
    t_.assign((size_t)4 * n_ + 4, Node{0, 0, 0, 0, Ring::one(), Ring::zero()});
    pos_.assign(n_, 0);
    if (n_ == 0) return;
    std::vector<int> idx(n_);
    std::iota(idx.begin(), idx.end(), 0);
    build(idx, 1, 0, n_, 0);
  }

  int size() const { return n_; }

  // [l, r) x [d, u) の点 i に v -> a v + b を当てる。
  void apply(int l, int d, int r, int u, W a, W b) {
    if (l >= r || d >= u) return;
    apply_(1, l, d, r, u, a, b);
  }

  // 点 i の値。根から葉へ作用を配ってから葉の作用を当てる。
  // 作用は順序を持つ (affine) ので、配らずに経路を重ねると、先祖に新しい作用が
  // 積まれたときに順序が入れ替わる。
  W get(int i) {
    int path[64];
    int len = 0;
    for (int v = pos_[i]; v; v >>= 1) path[len++] = v;
    for (int j = len - 1; j >= 1; --j) push(path[j]);
    const Node &p = t_[path[0]];
    return Ring::add(Ring::mul(p.a, init_[i]), p.b);
  }

 private:
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    W a, b;              // 作用。恒等は (1, 0)
  };

  int n_ = 0;
  std::vector<Node> t_;
  std::vector<int> xs_, ys_;
  std::vector<W> init_;
  std::vector<int> pos_;

  // 節 v に (a, b) を重ねる。
  void apply_node(int v, W a, W b) {
    Node &p = t_[v];
    p.a = Ring::mul(a, p.a);
    p.b = Ring::add(Ring::mul(a, p.b), b);
  }

  void push(int v) {
    Node &p = t_[v];
    if (p.a == Ring::one() && p.b == Ring::zero()) return;
    apply_node(2 * v, p.a, p.b);
    apply_node(2 * v + 1, p.a, p.b);
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
    apply_(2 * v, l, d, r, u, a, b);
    apply_(2 * v + 1, l, d, r, u, a, b);
  }

  void build(std::vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      pos_[i] = v;
      t_[v] = Node{xs_[i], ys_[i], xs_[i] + 1, ys_[i] + 1, Ring::one(), Ring::zero()};
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
