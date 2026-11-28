#pragma once
// KD 木で解く。全点の座標は入力の時点で分かるので、後から足す点も葉だけ先に作っておく。
// 各節は外接矩形 (閉区間) と (重みの和 mod 998244353, 有効な点の数) と affine の遅延を持つ。
// タイプ 0 は葉を (w, 1) に、タイプ 1 は葉の和を w に書き換える。どちらも根から葉まで遅延を
// 配ってから書き、戻りに和と数を計算し直す。矩形の和と affine は、外接矩形が範囲に入る節で
// 丸ごと処理し、交わらない節を打ち切る。2 次元の KD 木なので、1 クエリの訪問節数は
// O(sqrt(点の数))。
#include <algorithm>
#include <array>
#include <vector>

struct Solver {
  static constexpr long long mod = 998244353;
  using i64 = long long;

  struct Node {
    int l, r, par, cnt;  // 子 (葉は -1)、親、有効な点の数
    i64 lx, ly, hx, hy;  // 外接矩形 (閉区間)
    i64 sum, a, b;       // 重みの和、遅延 affine。恒等は (1, 0)
  };

  int n0;   // 最初の点の数
  int m;    // 全点の数 (最初の点 + 後から足す点)
  int nodes = 0;
  std::vector<i64> xs, ys;  // 全点の座標
  std::vector<int> leaf;    // 点 -> 葉の節
  std::vector<Node> t;
  int root = -1;
  const std::vector<std::array<i64, 7>> &queries;
  std::vector<i64> ans;

  Solver(const std::vector<std::array<i64, 3>> &points,
         const std::vector<std::array<i64, 2>> &spots,
         const std::vector<std::array<i64, 7>> &queries_)
      : n0((int)points.size()), m(n0 + (int)spots.size()), queries(queries_) {
    xs.resize(m);
    ys.resize(m);
    std::vector<i64> w0(m, 0);
    std::vector<int> c0(m, 0);
    for (int i = 0; i < n0; ++i) {
      xs[i] = points[i][0];
      ys[i] = points[i][1];
      w0[i] = points[i][2] % mod;
      c0[i] = 1;
    }
    for (int i = 0; i < (int)spots.size(); ++i) {
      xs[n0 + i] = spots[i][0];
      ys[n0 + i] = spots[i][1];
    }
    t.assign(2 * m + 1, Node{});
    leaf.assign(m, -1);
    std::vector<int> idx(m);
    for (int i = 0; i < m; ++i) idx[i] = i;
    root = build(idx, 0, m, 0, -1, w0, c0);
  }

  const std::vector<i64> &answers() const { return ans; }

  void run() {
    ans.clear();
    int k = n0;
    for (const auto &e : queries) {
      const int kind = (int)e[0];
      if (kind == 0) {
        set_point(k++, e[3]);
      } else if (kind == 1) {
        set_point((int)e[1], e[2]);
      } else if (kind == 2) {
        ans.push_back(rect_sum(root, e[1], e[2], e[3], e[4]));
      } else {
        rect_affine(root, e[1], e[2], e[3], e[4], e[5], e[6]);
      }
    }
  }

 private:
  int build(std::vector<int> &idx, int l, int r, int dep, int par,
            const std::vector<i64> &w0, const std::vector<int> &c0) {
    const int v = nodes++;
    t[v].par = par;
    t[v].l = t[v].r = -1;
    t[v].a = 1;
    t[v].b = 0;
    if (r - l == 1) {
      const int i = idx[l];
      leaf[i] = v;
      t[v].lx = t[v].hx = xs[i];
      t[v].ly = t[v].hy = ys[i];
      t[v].sum = w0[i];
      t[v].cnt = c0[i];
      return v;
    }
    const int mid = (l + r) >> 1;
    if ((dep & 1) == 0)
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&](int i, int j) { return xs[i] < xs[j]; });
    else
      std::nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                       [&](int i, int j) { return ys[i] < ys[j]; });
    const int left = build(idx, l, mid, dep + 1, v, w0, c0);
    const int right = build(idx, mid, r, dep + 1, v, w0, c0);
    t[v].l = left;
    t[v].r = right;
    t[v].lx = std::min(t[left].lx, t[right].lx);
    t[v].ly = std::min(t[left].ly, t[right].ly);
    t[v].hx = std::max(t[left].hx, t[right].hx);
    t[v].hy = std::max(t[left].hy, t[right].hy);
    pull(v);
    return v;
  }

  void pull(int v) {
    t[v].sum = (t[t[v].l].sum + t[t[v].r].sum) % mod;
    t[v].cnt = t[t[v].l].cnt + t[t[v].r].cnt;
  }

  // 節 v の全部の重みを w -> a w + b にする。
  void apply(int v, i64 a, i64 b) {
    t[v].sum = (a * t[v].sum + b * t[v].cnt) % mod;
    t[v].a = a * t[v].a % mod;
    t[v].b = (a * t[v].b + b) % mod;
  }

  // 節 v の遅延を子に配る。
  void push(int v) {
    if (t[v].a == 1 && t[v].b == 0) return;
    if (t[v].l >= 0) {
      apply(t[v].l, t[v].a, t[v].b);
      apply(t[v].r, t[v].a, t[v].b);
    }
    t[v].a = 1;
    t[v].b = 0;
  }

  // [l, r) x [d, u) に入る点の重みの和。
  i64 rect_sum(int v, i64 l, i64 d, i64 r, i64 u) {
    const i64 vlx = t[v].lx, vly = t[v].ly, vhx = t[v].hx, vhy = t[v].hy;
    if (vhx < l || r <= vlx || vhy < d || u <= vly) return 0;
    if (l <= vlx && vhx < r && d <= vly && vhy < u) return t[v].sum;
    push(v);
    return (rect_sum(t[v].l, l, d, r, u) + rect_sum(t[v].r, l, d, r, u)) % mod;
  }

  // [l, r) x [d, u) に入る点の重みを w -> a w + b にする。
  void rect_affine(int v, i64 l, i64 d, i64 r, i64 u, i64 a, i64 b) {
    const i64 vlx = t[v].lx, vly = t[v].ly, vhx = t[v].hx, vhy = t[v].hy;
    if (vhx < l || r <= vlx || vhy < d || u <= vly) return;
    if (l <= vlx && vhx < r && d <= vly && vhy < u) {
      apply(v, a, b);
      return;
    }
    push(v);
    rect_affine(t[v].l, l, d, r, u, a, b);
    rect_affine(t[v].r, l, d, r, u, a, b);
    pull(v);
  }

  // 点 i の重みを w にする。タイプ 0 の有効化もこれで行う。
  void set_point(int i, i64 w) {
    const int v = leaf[i];
    int path[64];
    int len = 0;
    for (int x = v; x >= 0; x = t[x].par) path[len++] = x;
    for (int j = len - 1; j >= 0; --j) push(path[j]);
    t[v].sum = w % mod;
    t[v].cnt = 1;
    t[v].a = 1;
    t[v].b = 0;
    for (int j = 1; j < len; ++j) pull(path[j]);
  }
};
