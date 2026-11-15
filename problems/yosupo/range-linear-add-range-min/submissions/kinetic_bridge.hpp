#pragma once
#include "pj.hpp"
#include <bit>

// kinetic と bridge_lazy を合わせた形。節は kinetic と同じく、部分木の最小とそれを取る葉、時刻を前と後ろへどれだけ
// 進めてもその葉が最小のままか (melt) を持ち、作用が melt に収まる間は値をずらすだけで済ませる。
// 収まらなかった節は「最小が古い」とだけ記し、区間の最小で要るときに求め直す。求め直しは、まず kinetic と同じく古い
// 子へ再帰して 2 つの子から取り直す。古い節を BUDGET 個より多く見たらそれをやめ、bridge_lazy と同じ橋 (左右の子の
// 凸包の共通の接線) をたどって 1 本の道で最小の葉へ下りる。道の上の橋の傾きから、その葉が最小のままでいる幅も
// そのまま決まる。kinetic は時刻を行き来する入力で再帰が木の全体に広がるが、ここでは 1 節の求め直しが
// O(BUDGET + log N) で済む。
//
// 橋は bridge_lazy と同じく、区間に足したときは端の道の節に「橋が古い」印を付けるだけにし、橋をたどるときに、
// 古い子から先に探し直す。節の値 (橋の点と最小) には、その節に積んだ作用まで足しておく。
// Library Checker のケースでは、BUDGET = 32 で橋をたどる求め直しが 1 回も起きない (16 だと数回起き、そのたびに
// 溜まった古い橋を数万回探し直す)。
struct Solver {
  static constexpr i64 INF = 2'000'000'000'000'000'000LL;
#ifndef KB_BUDGET
#define KB_BUDGET 32
#endif
  static constexpr int BUDGET = KB_BUDGET;
  struct Pt {
    i64 y;
    i64 x;
  };
  struct Geo {
    Pt p, q;     // 橋。葉なら p = q でその点
    i64 lm, lk;  // 子へ配っていない作用 y += lm x + lk
  };
  struct Kin {
    i64 mn, ax;  // 部分木の最小と、それを取る葉の位置
    i64 fw, bw;  // 時刻を前へ fw、後ろへ bw まで進めても ax が最小のまま
  };
  int sz = 1, lg = 0;
  vector<Geo> g;
  vector<Kin> kc;
  vector<char> stale, ok;

  static i64 cross(const Pt &a, const Pt &b, const Pt &c) {  // (b - a) x (c - a)
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  }

  explicit Solver(const vector<i64> &a) {
    const int n = int(a.size());
    sz = int(bit_ceil(unsigned(max(n, 1)))), lg = countr_zero(unsigned(sz));
    g.assign(2 * sz, {{0, 0}, {0, 0}, 0, 0});
    kc.assign(2 * sz, {0, 0, INF, INF});
    stale.assign(2 * sz, 0), ok.assign(2 * sz, 1);
    for (int i = 0; i < sz; ++i) {
      const i64 y = i < n ? a[i] : 0;
      g[sz + i].p = g[sz + i].q = {y, i};
      kc[sz + i] = {y, i, INF, INF};
    }
    for (int v = sz - 1; v >= 1; --v) rebuild(v), pull(v);
  }

  void all_apply(int k, i64 m, i64 c) {
    Geo &x = g[k];
    x.p.y += m * x.p.x + c, x.q.y += m * x.q.x + c;
    Kin &h = kc[k];
    if (k >= sz) {
      h.mn += m * h.ax + c;
      return;
    }
    x.lm += m, x.lk += c;
    if (ok[k]) {
      if (m >= 0 ? m <= h.fw : -m <= h.bw) h.mn += m * h.ax + c, h.fw = min(INF, h.fw - m), h.bw = min(INF, h.bw + m);
      else ok[k] = 0;
    }
  }
  void push(int k) {
    if (g[k].lm || g[k].lk) {
      all_apply(2 * k, g[k].lm, g[k].lk), all_apply(2 * k + 1, g[k].lm, g[k].lk);
      g[k].lm = g[k].lk = 0;
    }
  }

  // 子の最小がどちらも新しく、k の作用を配り終えているときに、kinetic と同じく 2 つの子から最小を取る。
  void pull(int k) {
    const Kin &L = kc[2 * k], &R = kc[2 * k + 1];
    const bool lw = L.mn <= R.mn;
    const Kin &w = lw ? L : R, &o = lw ? R : L;
    Kin &h = kc[k];
    h.mn = w.mn, h.ax = w.ax;
    h.fw = min(L.fw, R.fw), h.bw = min(L.bw, R.bw);
    if (w.ax > o.ax) h.fw = min(h.fw, (o.mn - w.mn) / (w.ax - o.ax));
    if (o.ax > w.ax) h.bw = min(h.bw, (o.mn - w.mn) / (o.ax - w.ax));
    ok[k] = 1;
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
    Pt a = g[l].p, b = g[l].q, c = g[r].p, e = g[r].q;
    auto movel = [&](int f) {
      am += g[l].lm, ak += g[l].lk, l = 2 * l + f;
      a = g[l].p, b = g[l].q, a.y += am * a.x + ak, b.y += am * b.x + ak;
    };
    auto mover = [&](int f) {
      bm += g[r].lm, bk += g[r].lk, r = 2 * r + f;
      c = g[r].p, e = g[r].q, c.y += bm * c.x + bk, e.y += bm * e.x + bk;
    };
    while (l < sz || r < sz) {
      if (l < sz && cross(a, b, c) < 0) movel(0);
      else if (r < sz && cross(b, c, e) < 0) mover(1);
      else if (l >= sz) mover(0);
      else if (r >= sz) movel(1);
      else {
        const __int128 c1 = __int128(b.x - a.x) * (e.y - c.y) - __int128(b.y - a.y) * (e.x - c.x);
        const __int128 c2 = __int128(b.x - a.x) * (b.y - c.y) - __int128(b.y - a.y) * (b.x - c.x);
        const bool left = (c1 == 0 && c2 == 0) ? c.x < border : __int128(c.x) * c1 + __int128(e.x - c.x) * c2 < c1 * border;
        if (left) movel(1);
        else mover(0);
      }
    }
    g[k].p = a, g[k].q = c, stale[k] = 0;
  }

  // 橋をたどって節 k の最小の葉へ下り、その葉が最小のままでいる幅とともに k に記す。
  void descend(int k) {
    if (stale[k]) rebuild(k);
    i64 m = 0, c = 0, fw = INF, bw = INF;
    int i = k;
    while (i < sz) {
      const Geo &x = g[i];
      const i64 D = (x.q.y - x.p.y) + m * (x.q.x - x.p.x), W = x.q.x - x.p.x;
      // 左へ下りる (D >= 0) なら、時刻を後ろへ D / W まで戻してもそのまま。右なら、前へ (-D - 1) / W まで。
      if (D >= 0) bw = min(bw, D / W);
      else fw = min(fw, (-D - 1) / W);
      m += x.lm, c += x.lk, i = 2 * i + (D < 0);
    }
    kc[k] = {g[i].p.y + m * g[i].p.x + c, i - sz, fw, bw};
    ok[k] = 1;
  }

  // kinetic と同じく、古い子へ再帰して 2 つの子から取り直す。古い節を budget 個より多く見たらやめて偽を返す
  // (それまでに取り直した節はそのまま正しい)。
  bool fix(int k, int &budget) {
    if (ok[k]) return true;
    if (--budget < 0) return false;
    push(k);
    if (!fix(2 * k, budget) || !fix(2 * k + 1, budget)) return false;
    pull(k);
    return true;
  }

  // 節 k の最小を新しくする。k の祖先の作用は配り終えている。古い節が少なければ子から取り直し、多ければ橋をたどる。
  void ensure(int k) {
    if (ok[k]) return;
    int budget = BUDGET;
    if (!fix(k, budget)) descend(k);
  }

  void push_ends(int l, int r) {
    for (int i = lg; i >= 1; --i) {
      if (((l >> i) << i) != l) push(l >> i);
      if (((r >> i) << i) != r) push((r - 1) >> i);
    }
  }

  void add(int l, int r, i64 b, i64 c) {
    l += sz, r += sz;
    push_ends(l, r);
    for (int a = l, z = r; a < z; a >>= 1, z >>= 1) {
      if (a & 1) all_apply(a++, b, c);
      if (z & 1) all_apply(--z, b, c);
    }
    for (int i = 1; i <= lg; ++i) {
      if (((l >> i) << i) != l) {
        const int k = l >> i;
        stale[k] = 1;
        if (ok[2 * k] && ok[2 * k + 1]) pull(k);
        else ok[k] = 0;
      }
      if (((r >> i) << i) != r) {
        const int k = (r - 1) >> i;
        stale[k] = 1;
        if (ok[2 * k] && ok[2 * k + 1]) pull(k);
        else ok[k] = 0;
      }
    }
  }

  i64 minimum(int l, int r) {
    l += sz, r += sz;
    push_ends(l, r);
    i64 res = numeric_limits<i64>::max();
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) ensure(l), res = min(res, kc[l].mn), ++l;
      if (r & 1) --r, ensure(r), res = min(res, kc[r].mn);
    }
    return res;
  }
};
