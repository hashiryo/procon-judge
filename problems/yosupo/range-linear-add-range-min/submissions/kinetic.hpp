#pragma once
#include "pj.hpp"

// 両向きの kinetic segment tree。要素 i を傾き i の直線と見て、区間に b i + c を足すのを、その区間の時刻を b 進める
// ことと読む。節は部分木の最小とそれを取る葉の傾き、時刻を前と後ろへどれだけ進めても最小を取る葉が変わらないか
// (melt) を持つ。作用がその幅に収まる節は値をずらすだけで済ませ、収まらない節だけ子へ下りて作り直す。
// Library Checker の首位 (chaihf、t98slider) と同じ作り。
//
// 時刻が一方向にだけ進むなら作り直しの回数は償却で抑えられるが、b の符号が入れ替わると保証が無い。例えば a_i = -i に
// 全体へ b = 2 と b = -2 を交互に足すと、毎回すべての節で最小を取る葉が入れ替わり、1 回が O(N) になる。
struct Solver {
  static constexpr i64 INF = 2'000'000'000'000'000'000LL;
  struct Node {
    i64 mn, sl;  // 部分木の最小と、それを取る葉の傾き (葉の位置)
    i64 fw, bw;  // 時刻を前へ fw まで、後ろへ bw まで進めても最小を取る葉が変わらない
    i64 lb, lc;  // 子へ配っていない作用 b i + c
  };
  int n;
  vector<Node> d;

  explicit Solver(const vector<i64> &a) : n(int(a.size())), d(4 * a.size() + 4) { build(1, 0, n, a); }

  void pull(int v) {
    const Node &L = d[2 * v], &R = d[2 * v + 1];
    const bool lw = L.mn <= R.mn;
    const Node &w = lw ? L : R, &o = lw ? R : L;
    Node &c = d[v];
    c.mn = w.mn, c.sl = w.sl;
    c.fw = min(L.fw, R.fw), c.bw = min(L.bw, R.bw);
    if (w.sl > o.sl) c.fw = min(c.fw, (o.mn - w.mn) / (w.sl - o.sl));
    if (o.sl > w.sl) c.bw = min(c.bw, (o.mn - w.mn) / (o.sl - w.sl));
  }

  void build(int v, int lo, int hi, const vector<i64> &a) {
    if (hi - lo == 1) {
      d[v] = {a[lo], lo, INF, INF, 0, 0};
      return;
    }
    const int mid = (lo + hi) >> 1;
    build(2 * v, lo, mid, a), build(2 * v + 1, mid, hi, a);
    pull(v);
  }

  void add_node(int v, int lo, int hi, i64 b, i64 c) {
    Node &x = d[v];
    if (hi - lo == 1) {
      x.mn += b * x.sl + c;
      return;
    }
    if (b >= 0 ? b <= x.fw : -b <= x.bw) {
      x.mn += b * x.sl + c;
      x.fw = min(INF, x.fw - b), x.bw = min(INF, x.bw + b);
      x.lb += b, x.lc += c;
      return;
    }
    push(v, lo, hi);
    const int mid = (lo + hi) >> 1;
    add_node(2 * v, lo, mid, b, c), add_node(2 * v + 1, mid, hi, b, c);
    pull(v);
  }

  void push(int v, int lo, int hi) {
    const i64 b = d[v].lb, c = d[v].lc;
    if (b == 0 && c == 0) return;
    const int mid = (lo + hi) >> 1;
    add_node(2 * v, lo, mid, b, c), add_node(2 * v + 1, mid, hi, b, c);
    d[v].lb = 0, d[v].lc = 0;
  }

  void add_rec(int v, int lo, int hi, int l, int r, i64 b, i64 c) {
    if (r <= lo || hi <= l) return;
    if (l <= lo && hi <= r) return add_node(v, lo, hi, b, c);
    push(v, lo, hi);
    const int mid = (lo + hi) >> 1;
    add_rec(2 * v, lo, mid, l, r, b, c), add_rec(2 * v + 1, mid, hi, l, r, b, c);
    pull(v);
  }

  i64 min_rec(int v, int lo, int hi, int l, int r) {
    if (r <= lo || hi <= l) return INF;
    if (l <= lo && hi <= r) return d[v].mn;
    push(v, lo, hi);
    const int mid = (lo + hi) >> 1;
    return min(min_rec(2 * v, lo, mid, l, r), min_rec(2 * v + 1, mid, hi, l, r));
  }

  void add(int l, int r, i64 b, i64 c) { add_rec(1, 0, n, l, r, b, c); }

  i64 minimum(int l, int r) { return min_rec(1, 0, n, l, r); }
};
