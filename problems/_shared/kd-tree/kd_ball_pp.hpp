#pragma once
// 2 次元 KD 木で、円に入る点を数える / 列挙する / 最近傍を引く。今の Library の KDTree と同じく
// 1 点につき 1 ノードを置く形 (点を節に置き、左右の部分木に残りを分ける)。点を葉だけに置く形
// (kd_ball_2d.hpp) よりノードが半分で済み、同じ範囲を覆うのにたどるノードが少ない。
//
// 各節は外接矩形 (半開)、部分木の点の数、自分の点を持つ。円との距離の下限が半径の 2 乗を超える節を
// 打ち切り、上限が半径の 2 乗以下なら節をまとめて数える (列挙と最近傍では葉の点まで見る)。
#include <algorithm>
#include <numeric>
#include <vector>

class KdBallPP {
 public:
  KdBallPP() {}
  KdBallPP(const std::vector<int> &x, const std::vector<int> &y, const std::vector<int> &id) {
    n_ = (int)x.size();
    px_ = x;
    py_ = y;
    pid_ = id;
    t_.reserve(n_);
    if (n_ == 0) return;
    std::vector<int> idx(n_);
    std::iota(idx.begin(), idx.end(), 0);
    root_ = build(idx, 0, n_, 0);
  }

  int size() const { return n_; }

  long long count(int x, int y, long long r2) const {
    if (n_ == 0) return 0;
    long long ret = 0;
    int st[128];
    int sp = 0;
    st[sp++] = root_;
    while (sp) {
      const int v = st[--sp];
      const Node &p = t_[v];
      if (min_dist2(p, x, y) > r2) continue;
      if (max_dist2(p, x, y) <= r2) {
        ret += p.cnt;
        continue;
      }
      const long long dx = (long long)p.px - x, dy = (long long)p.py - y;
      if (dx * dx + dy * dy <= r2) ++ret;
      if (p.ch[0] >= 0) st[sp++] = p.ch[0];
      if (p.ch[1] >= 0) st[sp++] = p.ch[1];
    }
    return ret;
  }

  template <class F> void search(int x, int y, long long r2, F f) const {
    if (n_ == 0) return;
    int st[128];
    int sp = 0;
    st[sp++] = root_;
    while (sp) {
      const int v = st[--sp];
      const Node &p = t_[v];
      if (min_dist2(p, x, y) > r2) continue;
      const long long dx = (long long)p.px - x, dy = (long long)p.py - y;
      if (dx * dx + dy * dy <= r2) f(pid_[p.pt]);
      if (p.ch[0] >= 0) st[sp++] = p.ch[0];
      if (p.ch[1] >= 0) st[sp++] = p.ch[1];
    }
  }

  long long nearest2(int x, int y) const {
    if (n_ == 0) return -1;
    long long best = -1;
    near(root_, x, y, best);
    return best;
  }

 private:
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    int px, py;          // 自分の点の座標
    int ch[2];           // 子。無ければ -1
    int pt;              // 自分の点の番号
    int cnt;             // 部分木の点の数
  };

  int n_ = 0, root_ = -1;
  std::vector<Node> t_;
  std::vector<int> px_, py_, pid_;

  // [lo, hi) の外にあるときだけ正の距離。分岐を入れずに max で出す。
  static long long axis_dist(long long a, int lo, int hi) {
    const long long d = std::max((long long)lo - a, a - (long long)(hi - 1));
    return d > 0 ? d : 0;
  }
  static long long min_dist2(const Node &p, int x, int y) {
    const long long dx = axis_dist(x, p.lx, p.hx), dy = axis_dist(y, p.ly, p.hy);
    return dx * dx + dy * dy;
  }
  static long long max_dist2(const Node &p, int x, int y) {
    const long long dx = std::max(std::abs((long long)p.lx - x), std::abs((long long)p.hx - 1 - x));
    const long long dy = std::max(std::abs((long long)p.ly - y), std::abs((long long)p.hy - 1 - y));
    return dx * dx + dy * dy;
  }

  int build(std::vector<int> &idx, int l, int r, int dep) {
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
    const int a = build(idx, l, mid, dep + 1);
    const int b = build(idx, mid + 1, r, dep + 1);
    Node &p = t_[v];
    p.pt = i;
    p.px = px_[i];
    p.py = py_[i];
    p.ch[0] = a;
    p.ch[1] = b;
    p.lx = p.px;
    p.ly = p.py;
    p.hx = p.px + 1;
    p.hy = p.py + 1;
    p.cnt = 1;
    if (a >= 0) {
      const Node &c = t_[a];
      p.lx = std::min(p.lx, c.lx);
      p.ly = std::min(p.ly, c.ly);
      p.hx = std::max(p.hx, c.hx);
      p.hy = std::max(p.hy, c.hy);
      p.cnt += c.cnt;
    }
    if (b >= 0) {
      const Node &c = t_[b];
      p.lx = std::min(p.lx, c.lx);
      p.ly = std::min(p.ly, c.ly);
      p.hx = std::max(p.hx, c.hx);
      p.hy = std::max(p.hy, c.hy);
      p.cnt += c.cnt;
    }
    return v;
  }

  void near(int v, int x, int y, long long &best) const {
    const Node &p = t_[v];
    if (best >= 0 && min_dist2(p, x, y) >= best) return;
    const long long dx = (long long)p.px - x, dy = (long long)p.py - y;
    const long long d2 = dx * dx + dy * dy;
    if (best < 0 || d2 < best) best = d2;
    const int a = p.ch[0], b = p.ch[1];
    if (a < 0) {
      if (b >= 0) near(b, x, y, best);
      return;
    }
    if (b < 0) {
      near(a, x, y, best);
      return;
    }
    const long long da = min_dist2(t_[a], x, y), db = min_dist2(t_[b], x, y);
    if (da <= db) {
      near(a, x, y, best);
      near(b, x, y, best);
    } else {
      near(b, x, y, best);
      near(a, x, y, best);
    }
  }
};
