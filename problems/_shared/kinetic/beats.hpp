#pragma once
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

// 各要素が直線 (x_i, y_i) で、作用 y_i += a x_i + b (a >= 0) を区間に当て、区間の最大 (IsMax) か最小を引く
// kinetic segment tree。節は「部分木で勝っている直線とその値、値が変わりうるまでに足せる a の幅 (nxt)」を持ち、
// 作用が nxt に収まる間は値をずらすだけで済ませる。収まらなければ子へ配って降り、2 つの子から取り直す
// (Segment Tree Beats と同じ形)。maspy の ds/segtree/beats_kinetic.hpp と同じ持ち方で、a >= 0 が要る。
// 区間の最大は、節の値が常に今の値なので、覆う節の値を比べるだけでよい (遅延を配らなくてよい)。
// x は要素ごとに任意 (単調でなくてよい)。x, y, nxt, b は long long で、a x + b と y の差が 64 bit に収まる範囲で使う。
template <bool IsMax> struct KineticBeats {
  using i64 = long long;
  using i128 = __int128_t;
  static constexpr i64 INF = i64(4) << 60;
  struct Node {
    i64 x = 0, y = 0, nxt = INF;
    int idx = -1;  // -1 は空 (大きさを 2 の冪に揃えたときの余り)
  };
  int n = 0, sz = 1;
  std::vector<Node> d;
  std::vector<i64> la, lb;
  std::vector<unsigned char> has;

  void init(const std::vector<i64> &xs, const std::vector<i64> &ys) {
    n = int(xs.size());
    sz = 1;
    while (sz < std::max(n, 1)) sz <<= 1;
    d.assign(2 * sz, Node{});
    la.assign(sz, 0), lb.assign(sz, 0), has.assign(sz, 0);
    for (int i = 0; i < n; ++i) d[sz + i] = Node{xs[i], ys[i], INF, i};
    for (int k = sz - 1; k >= 1; --k) pull(k);
  }

  void set(int i, i64 x, i64 y) {
    // 道の節に積んだ作用を配ってから葉を置き換える。置き換える前の作用は新しい直線には掛からない。
    std::vector<int> path;
    path.reserve(32);
    for (int k = sz + i;; k >>= 1) {
      path.push_back(k);
      if (k == 1) break;
    }
    for (int t = int(path.size()) - 1; t >= 0; --t)
      if (path[t] < sz) push(path[t]);
    d[sz + i] = Node{x, y, INF, i};
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
    M.nxt = std::min(L.nxt, R.nxt);
    i64 num, den;
    if constexpr (IsMax) {
      if (W.x >= Z.x) {
        d[k] = M;
        return;
      }
      num = W.y - Z.y, den = Z.x - W.x;  // 敗者 Z の傾きが大きい。Z が勝つ最初の整数時刻まで
    } else {
      if (W.x <= Z.x) {
        d[k] = M;
        return;
      }
      num = Z.y - W.y, den = W.x - Z.x;  // 敗者 Z の傾きが小さい
    }
    i128 t = (i128)num / den + 1;
    if (t < (i128)M.nxt) M.nxt = (i64)t;
    d[k] = M;
  }

  bool try_apply(int k, i64 a, i64 b) {
    Node &v = d[k];
    if (v.idx < 0) return true;  // 空の節はずらすものが無い
    if (v.nxt <= a) return false;
    v.y += a * v.x + b;
    v.nxt -= a;
    if (k < sz) la[k] += a, lb[k] += b, has[k] = 1;
    return true;
  }

  void push(int k) {
    if (!has[k]) return;
    i64 a = la[k], b = lb[k];
    has[k] = 0, la[k] = 0, lb[k] = 0;
    bool p = try_apply(2 * k, a, b);
    bool q = try_apply(2 * k + 1, a, b);
    // 節が a を受け入れたとき a < 節の nxt <= 子の nxt なので、子も受け入れる。
    assert(p && q);
    (void)p, (void)q;
  }

  void add_rec(int k, int nl, int nr, int l, int r, i64 a, i64 b) {
    if (r <= nl || nr <= l) return;
    if (l <= nl && nr <= r && try_apply(k, a, b)) return;
    if (k >= sz) return;  // 葉は nxt = INF なので、ここへは来ない
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
