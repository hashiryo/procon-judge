#pragma once
// 期待出力を作る参照実装 (提出としても測る)。点を葉に置く KD 木を、Library も共通のヘッダも使わずに
// このファイルの中で書く。深さで x と y を交互に中央値で分け、各節は外接矩形 (半開) を持つ。
// 最近傍は、近い側の子から降り、外接矩形までの距離が今の最善以上になったら打ち切る。
// 小さい入力は brute.hpp と突き合わせてある (pj testdata crosscheck --brute brute.hpp)。
#include <algorithm>
#include <array>
#include <numeric>
#include <vector>
#include "pj.hpp"

struct Solver {
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    int lo, hi;          // 葉なら点の範囲 [lo, hi)。内部なら lo = -1
  };

  const vector<array<i64, 2>> &pts_;
  const vector<array<i64, 2>> &qs_;
  vector<i64> ans_;
  vector<Node> t_;
  vector<int> bx_, by_;

  Solver(const vector<array<i64, 2>> &pts, const vector<array<i64, 2>> &qs) : pts_(pts), qs_(qs) {}

  void run() {
    const int n = (int)pts_.size();
    bx_.resize(n);
    by_.resize(n);
    t_.assign((size_t)4 * n + 4, Node{0, 0, 0, 0, -1, -1});
    if (n > 0) {
      vector<int> idx(n);
      std::iota(idx.begin(), idx.end(), 0);
      build(idx, 1, 0, n, 0);
    }
    ans_.clear();
    ans_.reserve(qs_.size());
    for (const auto &q : qs_) {
      i64 best = -1;
      if (n > 0) search(1, (i64)q[0], (i64)q[1], best);
      ans_.push_back(best);
    }
  }

  const vector<i64> &answers() const { return ans_; }

 private:
  void build(vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      bx_[l] = (int)pts_[i][0];
      by_[l] = (int)pts_[i][1];
      t_[v] = Node{bx_[l], by_[l], bx_[l] + 1, by_[l] + 1, l, r};
      return;
    }
    const int mid = (l + r) >> 1;
    if ((dep & 1) == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [this](int a, int b) { return pts_[a][0] < pts_[b][0]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [this](int a, int b) { return pts_[a][1] < pts_[b][1]; });
    build(idx, 2 * v, l, mid, dep + 1);
    build(idx, 2 * v + 1, mid, r, dep + 1);
    Node &p = t_[v];
    const Node &lc = t_[2 * v], &rc = t_[2 * v + 1];
    p.lx = std::min(lc.lx, rc.lx);
    p.ly = std::min(lc.ly, rc.ly);
    p.hx = std::max(lc.hx, rc.hx);
    p.hy = std::max(lc.hy, rc.hy);
  }

  static i64 dist2(int x0, int y0, int x1, int y1) {
    const i64 dx = (i64)x0 - x1, dy = (i64)y0 - y1;
    return dx * dx + dy * dy;
  }

  static i64 box_dist2(const Node &p, i64 x, i64 y) {
    const i64 dx = x < p.lx ? p.lx - x : (x > p.hx - 1 ? x - (p.hx - 1) : 0);
    const i64 dy = y < p.ly ? p.ly - y : (y > p.hy - 1 ? y - (p.hy - 1) : 0);
    return dx * dx + dy * dy;
  }

  void search(int v, i64 x, i64 y, i64 &best) const {
    const Node &p = t_[v];
    if (best >= 0 && box_dist2(p, x, y) >= best) return;
    if (p.lo >= 0) {
      for (int j = p.lo; j < p.hi; ++j) {
        const i64 d2 = dist2(bx_[j], by_[j], (int)x, (int)y);
        if (best < 0 || d2 < best) best = d2;
      }
      return;
    }
    const i64 dl = box_dist2(t_[2 * v], x, y), dr = box_dist2(t_[2 * v + 1], x, y);
    if (dl <= dr) {
      search(2 * v, x, y, best);
      search(2 * v + 1, x, y, best);
    } else {
      search(2 * v + 1, x, y, best);
      search(2 * v, x, y, best);
    }
  }
};
