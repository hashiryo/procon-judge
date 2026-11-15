#pragma once
#include "pj.hpp"
#include <bit>

// bridge と同じく、点 (i, a_i) の下側の凸包を節ごとの橋 (左右の子の凸包の共通の接線の 2 点) で持つ二分木。
// Library Checker の wery0 の提出 (371003) と同じく、次の 2 つを変える。
// (1) 区間に足したときは、端の道の節に「橋が古い」印を付けるだけにする。橋は、区間の最小でその節が要るときに、
//     古い子から先に探し直す。古い節の祖先はみな古いので、印の無い節の子孫はみな新しい。
// (2) 節の橋の点に、その節の作用まで足しておく。作用は節に積んだまま、子へ配るのは橋を探し直すときと、区間の最小で
//     端の道を下りるときだけ。区間の最小は、端の道を根から配ってから、覆う節ごとに橋の低い側へ下りる。
struct Solver {
  struct Pt {
    i64 y;
    i64 x;
  };
  struct Node {
    Pt p, q;     // 橋。葉なら p = q でその点。節に積んだ作用まで足した値
    i64 lm, lk;  // 子へ配っていない作用 y += lm x + lk
  };
  int sz = 1, lg = 0;
  vector<Node> d;
  vector<char> stale;

  static i64 cross(const Pt &a, const Pt &b, const Pt &c) {  // (b - a) x (c - a)
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  }

  explicit Solver(const vector<i64> &a) {
    const int n = int(a.size());
    sz = int(bit_ceil(unsigned(max(n, 1)))), lg = countr_zero(unsigned(sz));
    d.assign(2 * sz, {{0, 0}, {0, 0}, 0, 0});
    stale.assign(2 * sz, 0);
    for (int i = 0; i < sz; ++i) d[sz + i].p = d[sz + i].q = {i < n ? a[i] : 0, i};
    for (int v = sz - 1; v >= 1; --v) rebuild(v);
  }

  void all_apply(int k, i64 m, i64 c) {
    Node &x = d[k];
    x.p.y += m * x.p.x + c, x.q.y += m * x.q.x + c;
    if (k < sz) x.lm += m, x.lk += c;
  }
  void push(int k) {
    if (d[k].lm || d[k].lk) {
      all_apply(2 * k, d[k].lm, d[k].lk), all_apply(2 * k + 1, d[k].lm, d[k].lk);
      d[k].lm = d[k].lk = 0;
    }
  }

  void rebuild(int k) {
    push(k);
    int l = 2 * k, r = 2 * k + 1;
    if (stale[l]) rebuild(l);
    if (stale[r]) rebuild(r);
    i64 border = r;  // 右の子の最初の葉の位置
    while (border < sz) border <<= 1;
    border -= sz;
    i64 am = 0, ak = 0, bm = 0, bk = 0;  // 左と右の道で、下りるときに通り過ぎた節の作用の和
    Pt a = d[l].p, b = d[l].q, c = d[r].p, e = d[r].q;
    auto movel = [&](int f) {
      am += d[l].lm, ak += d[l].lk, l = 2 * l + f;
      a = d[l].p, b = d[l].q, a.y += am * a.x + ak, b.y += am * b.x + ak;
    };
    auto mover = [&](int f) {
      bm += d[r].lm, bk += d[r].lk, r = 2 * r + f;
      c = d[r].p, e = d[r].q, c.y += bm * c.x + bk, e.y += bm * e.x + bk;
    };
    while (l < sz || r < sz) {
      if (l < sz && cross(a, b, c) < 0) movel(0);
      else if (r < sz && cross(b, c, e) < 0) mover(1);
      else if (l >= sz) mover(0);
      else if (r >= sz) movel(1);
      else {
        // 直線 a b と直線 c e の交点が、右の子の始まりより左にあるか。
        const __int128 c1 = __int128(b.x - a.x) * (e.y - c.y) - __int128(b.y - a.y) * (e.x - c.x);
        const __int128 c2 = __int128(b.x - a.x) * (b.y - c.y) - __int128(b.y - a.y) * (b.x - c.x);
        const bool left = (c1 == 0 && c2 == 0) ? c.x < border : __int128(c.x) * c1 + __int128(e.x - c.x) * c2 < c1 * border;
        if (left) movel(1);
        else mover(0);
      }
    }
    d[k].p = a, d[k].q = c, stale[k] = 0;
  }

  // 節 k の全体の最小。k の祖先の作用は配り終えている。
  i64 subtree_min(int k) {
    if (stale[k]) rebuild(k);
    i64 m = 0, c = 0;
    while (k < sz) {
      const Node &x = d[k];
      const bool right = x.p.y - x.q.y > m * (x.q.x - x.p.x);
      m += x.lm, c += x.lk, k = 2 * k + right;
    }
    return d[k].p.y + m * d[k].p.x + c;
  }

  void add(int l, int r, i64 b, i64 c) {
    l += sz, r += sz;
    for (int a = l, z = r; a < z; a >>= 1, z >>= 1) {
      if (a & 1) all_apply(a++, b, c);
      if (z & 1) all_apply(--z, b, c);
    }
    for (int i = 1; i <= lg; ++i) {
      if (((l >> i) << i) != l) stale[l >> i] = 1;
      if (((r >> i) << i) != r) stale[(r - 1) >> i] = 1;
    }
  }

  i64 minimum(int l, int r) {
    l += sz, r += sz;
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i);
      if (((r >> i) << i) != r) push((r - 1) >> i);
    }
    i64 res = numeric_limits<i64>::max();
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) res = min(res, subtree_min(l++));
      if (r & 1) res = min(res, subtree_min(--r));
    }
    return res;
  }
};
