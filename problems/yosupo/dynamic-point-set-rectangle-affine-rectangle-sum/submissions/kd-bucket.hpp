#pragma once
// 2 次元 KD 木で、葉を 8 個までの点の束にする形。節の添字はセグメント木と同じ (子は 2v と 2v+1) で、
// 内部の節は外接矩形と (和, 数) と affine の遅延を持ち、葉は束の点の範囲 [lo, hi) を持つ。
// 点を 1 つずつ葉にする形より節が 1/8 になり、木が浅く、節がキャッシュに載りやすい。
// 問いの範囲が節の外接矩形を丸ごと覆えばその節をまとめて処理し、葉まで来たら束の点を 1 つずつ見る。
#include <algorithm>
#include <array>
#include <vector>
#include "pj.hpp"

struct KDBucketEngine {
  using u32 = unsigned;
  using u64 = unsigned long long;
  static constexpr u32 MOD = 998244353;
  static constexpr int B = 8;

  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    u32 sum, cnt;        // 重みの和、有効な点の数
    u32 a, b;            // 遅延 affine
    int lo, hi;          // 葉なら束の点の範囲 [lo, hi)。内部なら lo = -1
  };

  int m = 0, n0 = 0;
  std::vector<Node> t;
  std::vector<int> bx, by;  // 束の順に並べた点の座標
  std::vector<u32> bw;      // 束の順の生の重み (節の遅延は掛かっていない)
  std::vector<u32> bc;      // 有効か
  std::vector<int> leaf_of, leaf_pos;

  KDBucketEngine(const std::vector<std::array<i64, 3>> &pts,
                 const std::vector<std::array<i64, 2>> &spots) {
    n0 = (int)pts.size();
    m = n0 + (int)spots.size();
    std::vector<int> xs(m), ys(m);
    std::vector<u32> w0(m, 0), c0(m, 0);
    for (int i = 0; i < n0; ++i) xs[i] = (int)pts[i][0], ys[i] = (int)pts[i][1], w0[i] = (u32)(pts[i][2] % MOD), c0[i] = 1;
    for (int i = 0; i < (int)spots.size(); ++i) xs[n0 + i] = spots[i][0], ys[n0 + i] = spots[i][1];
    t.assign((size_t)4 * ((m + B - 1) / B + 1) + 8, Node{0, 0, 0, 0, 0, 0, 1, 0, -1, -1});
    std::vector<int> idx(m);
    for (int i = 0; i < m; ++i) idx[i] = i;
    bx.resize(m);
    by.resize(m);
    bw.resize(m);
    bc.resize(m);
    leaf_of.resize(m);
    leaf_pos.resize(m);
    build(idx, xs, ys, w0, c0, 1, 0, m, 0);
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
  void build(std::vector<int> &idx, const std::vector<int> &xs, const std::vector<int> &ys,
             const std::vector<u32> &w0, const std::vector<u32> &c0, int v, int l, int r, int dep) {
    if (r - l <= B) {
      int mnx = xs[idx[l]], mxx = mnx, mny = ys[idx[l]], mxy = mny;
      u32 s = 0, c = 0;
      for (int j = l; j < r; ++j) {
        const int i = idx[j];
        bx[j] = xs[i];
        by[j] = ys[i];
        bw[j] = w0[i];
        bc[j] = c0[i];
        leaf_of[i] = v;
        leaf_pos[i] = j;
        mnx = std::min(mnx, xs[i]);
        mxx = std::max(mxx, xs[i]);
        mny = std::min(mny, ys[i]);
        mxy = std::max(mxy, ys[i]);
        s += w0[i];
        if (s >= MOD) s -= MOD;
        c += c0[i];
      }
      t[v] = Node{mnx, mny, mxx + 1, mxy + 1, s, c, 1, 0, l, r};
      return;
    }
    const int mid = (l + r) >> 1;
    if ((dep & 1) == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&xs](int p, int q) { return xs[p] < xs[q]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&ys](int p, int q) { return ys[p] < ys[q]; });
    build(idx, xs, ys, w0, c0, 2 * v, l, mid, dep + 1);
    build(idx, xs, ys, w0, c0, 2 * v + 1, mid, r, dep + 1);
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
    if (p.sum >= MOD) p.sum -= MOD;
    p.cnt = lc.cnt + rc.cnt;
  }

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

  // 葉 v の遅延を束の点の生の重みに配る。
  void flush_leaf(int v) {
    Node &p = t[v];
    if (p.a == 1 && p.b == 0) return;
    if (p.a == 1) {
      for (int j = p.lo; j < p.hi; ++j) {
        bw[j] += p.b;
        if (bw[j] >= MOD) bw[j] -= MOD;
      }
    } else {
      for (int j = p.lo; j < p.hi; ++j) bw[j] = (u32)(((u64)p.a * bw[j] + p.b) % MOD);
    }
    p.a = 1;
    p.b = 0;
  }

  u32 leaf_sum(int v) const {
    const Node &p = t[v];
    u32 s = 0;
    for (int j = p.lo; j < p.hi; ++j) {
      if (!bc[j]) continue;
      s += bw[j];
      if (s >= MOD) s -= MOD;
    }
    return s;
  }

  u32 rect_sum(int v, int l, int d, int r, int u) {
    const Node &p = t[v];
    if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly) return 0;
    if (l <= p.lx && p.hx <= r && d <= p.ly && p.hy <= u) return p.sum;
    if (p.lo >= 0) {
      // 葉。束の点を 1 つずつ見る。無効な点は足さない。
      const u32 rx = (u32)(r - l), ry = (u32)(u - d);
      u32 s = 0;
      for (int j = p.lo; j < p.hi; ++j) {
        if (!bc[j] || (u32)(bx[j] - l) >= rx || (u32)(by[j] - d) >= ry) continue;
        s += (u32)(((u64)p.a * bw[j] + p.b) % MOD);
        if (s >= MOD) s -= MOD;
      }
      return s;
    }
    push(v);
    const u32 s = rect_sum(2 * v, l, d, r, u) + rect_sum(2 * v + 1, l, d, r, u);
    return s >= MOD ? s - MOD : s;
  }

  void rect_affine(int v, int l, int d, int r, int u, u32 a, u32 b) {
    const Node &p = t[v];
    if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly) return;
    if (l <= p.lx && p.hx <= r && d <= p.ly && p.hy <= u) {
      apply(v, a, b);
      return;
    }
    if (p.lo >= 0) {
      flush_leaf(v);
      const u32 rx = (u32)(r - l), ry = (u32)(u - d);
      Node &q = t[v];
      for (int j = q.lo; j < q.hi; ++j)
        if ((u32)(bx[j] - l) < rx && (u32)(by[j] - d) < ry)
          bw[j] = (u32)(((u64)a * bw[j] + b) % MOD);
      q.sum = leaf_sum(v);
      return;
    }
    push(v);
    rect_affine(2 * v, l, d, r, u, a, b);
    rect_affine(2 * v + 1, l, d, r, u, a, b);
    pull(v);
  }

  void set_point(int i, u32 w) {
    const int v = leaf_of[i];
    int path[40];
    int len = 0;
    for (int x = v >> 1; x; x >>= 1) path[len++] = x;
    for (int j = len - 1; j >= 0; --j) push(path[j]);
    flush_leaf(v);
    const int j = leaf_pos[i];
    const bool was = bc[j] != 0;
    bw[j] = w;
    bc[j] = 1;
    t[v].sum = leaf_sum(v);
    if (!was) {
      ++t[v].cnt;
      for (int h = 0; h < len; ++h) ++t[path[h]].cnt;
    }
    for (int h = 0; h < len; ++h) pull_sum(path[h]);
  }

  void pull_sum(int v) {
    Node &q = t[v];
    const Node &lc = t[2 * v], &rc = t[2 * v + 1];
    q.sum = lc.sum + rc.sum;
    if (q.sum >= MOD) q.sum -= MOD;
  }
};

struct Solver {
  KDBucketEngine kd;
  const vector<array<i64, 7>> &qs;
  vector<i64> ans;

  Solver(const vector<array<i64, 3>> &points,
         const vector<array<i64, 2>> &spots,
         const vector<array<i64, 7>> &queries)
      : kd(points, spots), qs(queries) {}

  void run() {
    ans.clear();
    kd.run(qs, ans);
  }

  const vector<i64> &answers() const { return ans; }
};
