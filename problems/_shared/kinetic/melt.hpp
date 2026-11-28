#pragma once
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

// 各要素が直線 (x_i, y_i) で、作用 y_i += a x_i + b を区間に当て、区間の最大 (IsMax) か最小を引く kinetic segment
// tree の、イベントを使わない形。節は「部分木で勝っている直線とその値、時刻を前へ fw・後ろへ bw だけ動かしても勝ち続ける
// 幅」を持つ。作用が幅に収まる間は値をずらすだけで済ませ、収まらなければ子へ配って降りる。chaihf と t98slider の
// Library Checker の提出と同じ持ち方で、時刻を前後に行き来させると 1 回の作用が O(N) になりうる (計算量の保証は無い)。
// 区間の値は節の値が常に今の値なので、覆う節を比べるだけでよい。
template <bool IsMax> struct KineticMelt {
  using i64 = long long;
  using i128 = __int128_t;
  static constexpr i64 INF = i64(4) << 60;
  struct Node {
    i64 x = 0, y = 0, fw = INF, bw = INF;
    int idx = -1;
  };
  int n = 0, sz = 1;
  std::vector<Node> d;
  std::vector<i64> ea, eb;
  std::vector<unsigned char> has;

  void init(const std::vector<i64> &xs, const std::vector<i64> &ys) {
    n = int(xs.size());
    sz = 1;
    while (sz < std::max(n, 1)) sz <<= 1;
    d.assign(2 * sz, Node{});
    ea.assign(sz, 0), eb.assign(sz, 0), has.assign(sz, 0);
    for (int i = 0; i < n; ++i) d[sz + i] = Node{xs[i], ys[i], INF, INF, i};
    for (int k = sz - 1; k >= 1; --k) pull(k);
  }

  void set(int i, i64 x, i64 y) {
    std::vector<int> path;
    path.reserve(32);
    for (int k = sz + i;; k >>= 1) {
      path.push_back(k);
      if (k == 1) break;
    }
    for (int t = int(path.size()) - 1; t >= 0; --t)
      if (path[t] < sz) push(path[t]);
    d[sz + i] = Node{x, y, INF, INF, i};
    for (int v = (sz + i) >> 1; v >= 1; v >>= 1) pull(v);
  }

  void add(int l, int r, i64 a, i64 b) {
    if (l >= r) return;
    add_rec(1, 0, sz, l, r, a, b);
  }

  i64 get(int l, int r) { return get_rec(1, 0, sz, l, r); }
  i64 all() { return d[1].idx < 0 ? (IsMax ? -INF : INF) : d[1].y; }

 private:
  static bool better(const Node &A, const Node &B) {
    if (A.idx < 0) return false;
    if (B.idx < 0) return true;
    if constexpr (IsMax) return A.y > B.y || (A.y == B.y && A.idx < B.idx);
    else return A.y < B.y || (A.y == B.y && A.idx < B.idx);
  }

  void pull(int k) {
    const Node &L = d[2 * k], &R = d[2 * k + 1];
    if (L.idx < 0) {
      d[k] = R;
      return;
    }
    if (R.idx < 0) {
      d[k] = L;
      return;
    }
    bool lw = better(L, R);
    const Node &W = lw ? L : R, &Z = lw ? R : L;
    Node M = W;
    M.fw = std::min(L.fw, R.fw), M.bw = std::min(L.bw, R.bw);
    // 前へ d 進めると敗者 Z が勝つ最初の d (Z の傾きのほうが大きい場合だけ)。
    // 後ろへ d 戻すと敗者 Z が勝つ最初の d (Z の傾きのほうが小さい場合だけ)。
    i64 num, den;
    if constexpr (IsMax) {
      if (Z.x > W.x) {
        num = W.y - Z.y, den = Z.x - W.x;
        if (num < 0) num = 0;
        i128 t = (i128)num / den + 1;
        if (t < (i128)M.fw) M.fw = (i64)t;
      }
      if (Z.x < W.x) {
        num = W.y - Z.y, den = W.x - Z.x;
        if (num < 0) num = 0;
        i128 t = (i128)num / den + 1;
        if (t < (i128)M.bw) M.bw = (i64)t;
      }
    } else {
      if (Z.x < W.x) {
        num = Z.y - W.y, den = W.x - Z.x;
        if (num < 0) num = 0;
        i128 t = (i128)num / den + 1;
        if (t < (i128)M.fw) M.fw = (i64)t;
      }
      if (Z.x > W.x) {
        num = Z.y - W.y, den = Z.x - W.x;
        if (num < 0) num = 0;
        i128 t = (i128)num / den + 1;
        if (t < (i128)M.bw) M.bw = (i64)t;
      }
    }
    d[k] = M;
  }

  bool try_apply(int k, i64 a, i64 b) {
    Node &v = d[k];
    if (v.idx < 0) return true;
    if (a >= v.fw || -a >= v.bw) return false;
    v.y += a * v.x + b;
    v.fw -= a, v.bw += a;
    if (v.fw < 0) v.fw = 0;
    if (v.bw < 0) v.bw = 0;
    if (k < sz) ea[k] += a, eb[k] += b, has[k] = 1;
    return true;
  }

  void push(int k) {
    if (!has[k]) return;
    i64 a = ea[k], b = eb[k];
    has[k] = 0, ea[k] = 0, eb[k] = 0;
    bool p = try_apply(2 * k, a, b);
    bool q = try_apply(2 * k + 1, a, b);
    assert(p && q);
    (void)p, (void)q;
  }

  void add_rec(int k, int nl, int nr, int l, int r, i64 a, i64 b) {
    if (r <= nl || nr <= l) return;
    if (l <= nl && nr <= r && try_apply(k, a, b)) return;
    if (k >= sz) return;
    push(k);
    int m = (nl + nr) >> 1;
    add_rec(2 * k, nl, m, l, r, a, b);
    add_rec(2 * k + 1, m, nr, l, r, a, b);
    pull(k);
  }

  i64 get_rec(int k, int nl, int nr, int l, int r) {
    if (r <= nl || nr <= l || d[k].idx < 0) return IsMax ? -INF : INF;
    if (l <= nl && nr <= r) return d[k].y;
    push(k);  // 端で降りるときは子を今の時刻にそろえる
    int m = (nl + nr) >> 1;
    i64 A = get_rec(2 * k, nl, m, l, r), B = get_rec(2 * k + 1, m, nr, l, r);
    if constexpr (IsMax) return std::max(A, B);
    else return std::min(A, B);
  }
};
