#pragma once
// KD 木を、点の取得の座標の集合に対して作る。長方形の加算は、外接矩形が範囲に入る節に
// 遅延を積むだけで、点の取得は葉から根までの遅延を足す。遅延を配る必要がない
// (取得のときに祖先の遅延はそのまま効く) ので、加算は O(sqrt(点の数))、取得は O(log)。
// 加算が多いケースでは 2 次元 BIT に劣り、取得が多いケースでは勝つ。
#include <algorithm>
#include <array>
#include <numeric>
#include <vector>
#include "pj.hpp"

struct Solver {
  Solver(const vector<array<i64, 5>> &rect, const vector<array<i64, 6>> &query)
      : rect_(rect), query_(query) {}

  void run();
  const vector<i64> &answers() const { return ans_; }

 private:
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    i64 laz;             // この節の全体に足された値
  };

  const vector<array<i64, 5>> &rect_;
  const vector<array<i64, 6>> &query_;
  vector<Node> t;
  vector<int> xs, ys;  // 取得の点の座標
  vector<int> leaf;    // 取得 -> 葉の節
  vector<i64> ans_;

  void build(vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      leaf[i] = v;
      t[v] = Node{xs[i], ys[i], xs[i] + 1, ys[i] + 1, 0};
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
    Node &p = t[v];
    const Node &lc = t[2 * v], &rc = t[2 * v + 1];
    p.lx = std::min(lc.lx, rc.lx);
    p.ly = std::min(lc.ly, rc.ly);
    p.hx = std::max(lc.hx, rc.hx);
    p.hy = std::max(lc.hy, rc.hy);
    p.laz = 0;
  }

  void rect_add(int v, int l, int d, int r, int u, i64 w) {
    const int lx = t[v].lx, ly = t[v].ly, hx = t[v].hx, hy = t[v].hy;
    if (hx <= l || r <= lx || hy <= d || u <= ly) return;
    if (l <= lx && hx <= r && d <= ly && hy <= u) {
      t[v].laz += w;
      return;
    }
    rect_add(2 * v, l, d, r, u, w);
    rect_add(2 * v + 1, l, d, r, u, w);
  }
};

inline void Solver::run() {
  int q1 = 0;
  for (const auto &e : query_)
    if (e[0] == 1) ++q1;
  if (q1 > 0) {
    xs.resize(q1);
    ys.resize(q1);
    int k = 0;
    for (const auto &e : query_)
      if (e[0] == 1) xs[k] = (int)e[1], ys[k] = (int)e[2], ++k;
    t.assign((size_t)4 * q1 + 4, Node{0, 0, 0, 0, 0});
    leaf.assign(q1, 0);
    vector<int> idx(q1);
    std::iota(idx.begin(), idx.end(), 0);
    build(idx, 1, 0, q1, 0);
  }

  for (const auto &e : rect_) rect_add(1, (int)e[0], (int)e[1], (int)e[2], (int)e[3], e[4]);
  ans_.clear();
  ans_.reserve(q1);
  int k = 0;
  for (const auto &e : query_) {
    if (e[0] == 0) {
      rect_add(1, (int)e[1], (int)e[2], (int)e[3], (int)e[4], e[5]);
    } else {
      i64 s = 0;
      for (int v = leaf[k++]; v; v >>= 1) s += t[v].laz;
      ans_.push_back(s);
    }
  }
}
