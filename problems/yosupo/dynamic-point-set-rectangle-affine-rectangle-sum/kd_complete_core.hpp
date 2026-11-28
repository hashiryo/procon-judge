#pragma once
// 2 次元 KD 木の核 (yosupo-dynamic-point-set-rectangle-affine-rectangle-sum 用)。
// 点を葉に置く 2 分木で、節の添字はセグメント木と同じ (節 v の子は 2v と 2v+1)。
// 子への添字を持たないので、節をたどる間の読み出しが減る。
//
// 各節は外接矩形 (半開) と (重みの和 mod、有効な点の数) と affine の遅延を持つ。
// 問いの範囲が外接矩形を丸ごと覆えばその節の和を返し、交わらなければ打ち切る。
// 点の集合は入力の時点で全部分かるので、あとから足す点も葉だけ先に作っておく。
//
// SPLIT_WIDE が true なら、外接矩形の広い方の軸で分ける。false なら深さで交互に分ける。
// 広い方で分けると節の外接矩形が正方形に近づき、問いの範囲と交わる節が減る。
#include <algorithm>
#include <array>
#include <vector>
#include "pj.hpp"

template <bool SPLIT_WIDE> struct KDComplete {
  using u32 = unsigned;
  using u64 = unsigned long long;
  static constexpr u32 MOD = 998244353;

  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    u32 sum, cnt;        // 重みの和、有効な点の数
    u32 a, b;            // 遅延 affine。恒等は (1, 0)
  };

  int m = 0;                // 全点の数
  int n0 = 0;               // 最初の点の数
  std::vector<Node> t;
  std::vector<int> xs, ys;  // 全点の座標
  std::vector<u32> w0;      // 最初の重み
  std::vector<char> on0;    // 最初に有効か
  std::vector<int> pos;     // 点 -> 葉の節

  KDComplete() {}
  KDComplete(const std::vector<std::array<i64, 3>> &pts,
             const std::vector<std::array<i64, 2>> &spots) {
    n0 = (int)pts.size();
    m = n0 + (int)spots.size();
    xs.resize(m);
    ys.resize(m);
    w0.assign(m, 0);
    on0.assign(m, 0);
    for (int i = 0; i < n0; ++i) {
      xs[i] = (int)pts[i][0];
      ys[i] = (int)pts[i][1];
      w0[i] = (u32)(pts[i][2] % MOD);
      on0[i] = 1;
    }
    for (int i = 0; i < (int)spots.size(); ++i) {
      xs[n0 + i] = (int)spots[i][0];
      ys[n0 + i] = (int)spots[i][1];
    }
    t.assign((size_t)4 * m + 4, Node{0, 0, 0, 0, 0, 0, 1, 0});
    pos.resize(m);
    std::vector<int> idx(m);
    for (int i = 0; i < m; ++i) idx[i] = i;
    build(idx, 1, 0, m, 0);
  }

  void run(const std::vector<std::array<i64, 7>> &qs, std::vector<i64> &ans) {
    int k = n0;
    for (const auto &e : qs) {
      const int kind = (int)e[0];
      if (kind == 0) set_point(k++, (u32)(e[3] % MOD));
      else if (kind == 1) set_point((int)e[1], (u32)(e[2] % MOD));
      else if (kind == 2) ans.push_back(rect_sum(1, (int)e[1], (int)e[2], (int)e[3], (int)e[4]));
      else rect_affine(1, (int)e[1], (int)e[2], (int)e[3], (int)e[4], (u32)(e[5] % MOD), (u32)(e[6] % MOD));
    }
  }

 private:
  void build(std::vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      pos[i] = v;
      t[v] = Node{xs[i], ys[i], xs[i] + 1, ys[i] + 1, w0[i], (u32)(on0[i] != 0), 1, 0};
      return;
    }
    const int mid = (l + r) >> 1;
    int dim;
    if constexpr (SPLIT_WIDE) {
      int mnx = xs[idx[l]], mxx = mnx, mny = ys[idx[l]], mxy = mny;
      for (int j = l + 1; j < r; ++j) {
        const int x = xs[idx[j]], y = ys[idx[j]];
        if (x < mnx) mnx = x;
        if (mxx < x) mxx = x;
        if (y < mny) mny = y;
        if (mxy < y) mxy = y;
      }
      dim = (mxx - mnx >= mxy - mny) ? 0 : 1;
    } else {
      dim = dep & 1;
    }
    const std::vector<int> &cx = xs, &cy = ys;
    if (dim == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cx](int p, int q) { return cx[p] < cx[q]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&cy](int p, int q) { return cy[p] < cy[q]; });
    build(idx, 2 * v, l, mid, dep + 1);
    build(idx, 2 * v + 1, mid, r, dep + 1);
    pull_bbox(v);
    pull(v);
  }

  // 外接矩形は構築のあと変わらないので、直すのは和と数だけ。
  void pull(int v) {
    const Node &lc = t[2 * v], &rc = t[2 * v + 1];
    Node &p = t[v];
    p.sum = lc.sum + rc.sum;
    if (p.sum >= MOD) p.sum -= MOD;
    p.cnt = lc.cnt + rc.cnt;
  }

  void pull_bbox(int v) {
    const Node &lc = t[2 * v], &rc = t[2 * v + 1];
    Node &p = t[v];
    p.lx = std::min(lc.lx, rc.lx);
    p.ly = std::min(lc.ly, rc.ly);
    p.hx = std::max(lc.hx, rc.hx);
    p.hy = std::max(lc.hy, rc.hy);
  }

  // 節 v の全部の重みを a w + b にする。
  void apply(int v, u32 a, u32 b) {
    Node &p = t[v];
    p.sum = (u32)(((u64)a * p.sum + (u64)b * p.cnt) % MOD);
    p.a = (u32)((u64)a * p.a % MOD);
    p.b = (u32)(((u64)a * p.b + b) % MOD);
  }

  void push(int v) {
    Node &p = t[v];
    if (p.a == 1 && p.b == 0) return;
    apply(2 * v, p.a, p.b);
    apply(2 * v + 1, p.a, p.b);
    p.a = 1;
    p.b = 0;
  }

  u32 rect_sum(int v, int l, int d, int r, int u) {
    const int lx = t[v].lx, ly = t[v].ly, hx = t[v].hx, hy = t[v].hy;
    if (hx <= l || r <= lx || hy <= d || u <= ly) return 0;
    if (l <= lx && hx <= r && d <= ly && hy <= u) return t[v].sum;
    push(v);
    const u32 s = rect_sum(2 * v, l, d, r, u) + rect_sum(2 * v + 1, l, d, r, u);
    return s >= MOD ? s - MOD : s;
  }

  void rect_affine(int v, int l, int d, int r, int u, u32 a, u32 b) {
    const int lx = t[v].lx, ly = t[v].ly, hx = t[v].hx, hy = t[v].hy;
    if (hx <= l || r <= lx || hy <= d || u <= ly) return;
    if (l <= lx && hx <= r && d <= ly && hy <= u) {
      apply(v, a, b);
      return;
    }
    push(v);
    rect_affine(2 * v, l, d, r, u, a, b);
    rect_affine(2 * v + 1, l, d, r, u, a, b);
    pull(v);
  }

  // 点 i の重みを w にする。タイプ 0 の有効化もこれで行う。
  void set_point(int i, u32 w) {
    const int v = pos[i];
    int path[40];
    int len = 0;
    for (int x = v >> 1; x; x >>= 1) path[len++] = x;
    for (int j = len - 1; j >= 0; --j) push(path[j]);
    t[v].sum = w;
    t[v].cnt = 1;
    t[v].a = 1;
    t[v].b = 0;
    for (int j = 0; j < len; ++j) pull(path[j]);
  }
};
