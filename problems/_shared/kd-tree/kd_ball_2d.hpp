#pragma once
// 2 次元 KD 木で、点を中心とする円に入る点を数える / 列挙する。点を葉に置く完全 2 分木
// (子は 2v と 2v+1) で、深さで x と y を交互に、中央値で分ける。
// 各節は外接矩形 (半開) と部分木の点の数を持ち、円との距離の下限が半径の 2 乗を超える節を
// 打ち切り、上限が半径の 2 乗以下なら節をまとめて数える (列挙では葉まで降りる)。
#include <algorithm>
#include <numeric>
#include <vector>

class KdBall2D {
 public:
  KdBall2D() {}
  KdBall2D(const std::vector<int> &x, const std::vector<int> &y, const std::vector<int> &id) {
    n_ = (int)x.size();
    px_ = x;
    py_ = y;
    pid_ = id;
    bx_.resize(n_);
    by_.resize(n_);
    bid_.resize(n_);
    t_.assign((size_t)4 * n_ + 4, Node{0, 0, 0, 0, 0, -1, -1});
    if (n_ == 0) return;
    std::vector<int> idx(n_);
    std::iota(idx.begin(), idx.end(), 0);
    build(idx, 1, 0, n_, 0);
  }

  int size() const { return n_; }

  // 中心 (x, y)、半径の 2 乗 r2 の円に入る点の数。
  long long count(int x, int y, long long r2) const {
    if (n_ == 0) return 0;
    long long ret = 0;
    int st[128];
    int sp = 0;
    st[sp++] = 1;
    while (sp) {
      const int v = st[--sp];
      const Node &p = t_[v];
      if (min_dist2(p, x, y) > r2) continue;
      if (max_dist2(p, x, y) <= r2) {
        ret += p.cnt;
        continue;
      }
      if (p.lo >= 0) {
        for (int j = p.lo; j < p.hi; ++j) {
          const long long dx = (long long)bx_[j] - x, dy = (long long)by_[j] - y;
          if (dx * dx + dy * dy <= r2) ++ret;
        }
        continue;
      }
      st[sp++] = 2 * v;
      st[sp++] = 2 * v + 1;
    }
    return ret;
  }

  // 円に入る点の番号を f に渡す。
  template <class F> void search(int x, int y, long long r2, F f) const {
    if (n_ == 0) return;
    int st[128];
    int sp = 0;
    st[sp++] = 1;
    while (sp) {
      const int v = st[--sp];
      const Node &p = t_[v];
      if (min_dist2(p, x, y) > r2) continue;
      if (p.lo >= 0) {
        for (int j = p.lo; j < p.hi; ++j) {
          const long long dx = (long long)bx_[j] - x, dy = (long long)by_[j] - y;
          if (dx * dx + dy * dy <= r2) f(bid_[j]);
        }
        continue;
      }
      st[sp++] = 2 * v;
      st[sp++] = 2 * v + 1;
    }
  }

 private:
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    int cnt;             // 部分木の点の数
    int lo, hi;          // 葉なら点の範囲 [lo, hi)。内部なら lo = -1
  };

  int n_ = 0;
  std::vector<Node> t_;
  std::vector<int> px_, py_, pid_;  // 入力の順の点
  std::vector<int> bx_, by_, bid_;  // 葉の順に並べた点

  static long long clamp_dist(long long a, int lo, int hi) {
    if (a < lo) return lo - a;
    if (a > hi - 1) return a - (hi - 1);
    return 0;
  }
  static long long min_dist2(const Node &p, int x, int y) {
    const long long dx = clamp_dist(x, p.lx, p.hx), dy = clamp_dist(y, p.ly, p.hy);
    return dx * dx + dy * dy;
  }
  static long long max_dist2(const Node &p, int x, int y) {
    const long long dx = std::max(std::abs((long long)p.lx - x), std::abs((long long)p.hx - 1 - x));
    const long long dy = std::max(std::abs((long long)p.ly - y), std::abs((long long)p.hy - 1 - y));
    return dx * dx + dy * dy;
  }

  void build(std::vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      bx_[l] = px_[i];
      by_[l] = py_[i];
      bid_[l] = pid_[i];
      t_[v] = Node{bx_[l], by_[l], bx_[l] + 1, by_[l] + 1, 1, l, r};
      return;
    }
    const int mid = (l + r) >> 1;
    const std::vector<int> &cx = px_, &cy = py_;
    if ((dep & 1) == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cx](int a, int b) { return cx[a] < cx[b]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cy](int a, int b) { return cy[a] < cy[b]; });
    build(idx, 2 * v, l, mid, dep + 1);
    build(idx, 2 * v + 1, mid, r, dep + 1);
    Node &p = t_[v];
    const Node &lc = t_[2 * v], &rc = t_[2 * v + 1];
    p.lx = std::min(lc.lx, rc.lx);
    p.ly = std::min(lc.ly, rc.ly);
    p.hx = std::max(lc.hx, rc.hx);
    p.hy = std::max(lc.hy, rc.hy);
    p.cnt = lc.cnt + rc.cnt;
  }
};
