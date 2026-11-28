#pragma once
// 2 次元 KD 木。点を葉に置く 2 分木で、節の添字はセグメント木と同じ (子は 2v と 2v+1)。
// 深さで x と y を交互に、中央値で分ける。各節は外接矩形 (半開) と部分木の重みの和を持つ。
// 点の加算は、構築で受け取った点の添字の葉に足して根まで直す。長方形の和は、
// 外接矩形が範囲に入る節の和を足し、交わらない節を打ち切る。1 クエリは O(sqrt(点の数))。
#include <algorithm>
#include <array>
#include <numeric>
#include <vector>
#include "pj.hpp"

struct Solver {
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    i64 sum;             // 部分木の重みの和
  };

  int n0 = 0, m = 0;
  std::vector<Node> t;
  std::vector<int> xs, ys;
  std::vector<i64> w0;
  std::vector<int> pos;  // 点 -> 葉の節

  Solver(const vector<array<i64, 3>> &points, const vector<array<int, 2>> &spots) {
    n0 = (int)points.size();
    m = n0 + (int)spots.size();
    xs.resize(m);
    ys.resize(m);
    w0.assign(m, 0);
    for (int i = 0; i < n0; ++i) xs[i] = (int)points[i][0], ys[i] = (int)points[i][1], w0[i] = points[i][2];
    for (int i = 0; i < (int)spots.size(); ++i) xs[n0 + i] = spots[i][0], ys[n0 + i] = spots[i][1];
    t.assign((size_t)4 * m + 4, Node{0, 0, 0, 0, 0});
    pos.resize(m);
    std::vector<int> idx(m);
    std::iota(idx.begin(), idx.end(), 0);
    build(idx, 1, 0, m, 0);
  }

  void add(int, int, i64 w) { add_at(n0 + n_add_++, w); }

  i64 rect_sum(int l, int d, int r, int u) const { return rect_sum_(1, l, d, r, u); }

 private:
  int n_add_ = 0;

  void build(std::vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      pos[i] = v;
      t[v] = Node{xs[i], ys[i], xs[i] + 1, ys[i] + 1, w0[i]};
      return;
    }
    const int mid = (l + r) >> 1;
    if ((dep & 1) == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [this](int p, int q) { return xs[p] < xs[q]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [this](int p, int q) { return ys[p] < ys[q]; });
    build(idx, 2 * v, l, mid, dep + 1);
    build(idx, 2 * v + 1, mid, r, dep + 1);
    pull(v);
  }

  void pull(int v) {
    const Node &lc = t[2 * v], &rc = t[2 * v + 1];
    Node &p = t[v];
    p.lx = std::min(lc.lx, rc.lx);
    p.ly = std::min(lc.ly, rc.ly);
    p.hx = std::max(lc.hx, rc.hx);
    p.hy = std::max(lc.hy, rc.hy);
    p.sum = lc.sum + rc.sum;
  }

  void add_at(int i, i64 w) {
    for (int v = pos[i]; v; v >>= 1) t[v].sum += w;
  }

  i64 rect_sum_(int v, int l, int d, int r, int u) const {
    const Node &p = t[v];
    if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly) return 0;
    if (l <= p.lx && p.hx <= r && d <= p.ly && p.hy <= u) return p.sum;
    return rect_sum_(2 * v, l, d, r, u) + rect_sum_(2 * v + 1, l, d, r, u);
  }
};
