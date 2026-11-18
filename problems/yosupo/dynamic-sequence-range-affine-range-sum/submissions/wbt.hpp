#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"

// 配列の pool に節を置いた重み平衡木 (どの節も値を持つ形)。重みは大きさ + 1 で、左右の重みの比が 3 を超えないように保つ。split と
// 併合は join (真ん中の節を 1 つ挟んで 2 つの木をつなぎ、重い方の背を下りて回転で直す、Blelloch らの Just Join の形) で書く。
// 区間の和と 1 次関数の作用は、形を変えずに根から下りて覆う部分木に当てる (今の Library の WeightBalancedTree と同じ)。反転だけ
// split を 2 回して真ん中の木に印を付け、併合で戻す。1 次関数の作用と反転は遅延で持つ。
struct Solver {
  using Z = ZMod<998244353>;
  struct Node {
    int l, r, sz;
    Z val, sum, b, c;
    bool rev, lz;
  };
  vector<Node> t;
  int root = 0;
  int make(Z v) {
    t.push_back({0, 0, 1, v, v, Z(1), Z(0), false, false});
    return (int)t.size() - 1;
  }
  int w(int x) const { return t[x].sz + 1; }
  static bool bal(long long wa, long long wb) { return 3 * wa >= wb && 3 * wb >= wa; }
  void aff(int x, Z b, Z c) {
    Node &n = t[x];
    n.val = b * n.val + c, n.sum = b * n.sum + c * Z(n.sz);
    n.b = b * n.b, n.c = b * n.c + c, n.lz = true;
  }
  void tog(int x) { swap(t[x].l, t[x].r), t[x].rev ^= 1; }
  void push(int x) {
    Node &n = t[x];
    if (n.rev) {
      if (n.l) tog(n.l);
      if (n.r) tog(n.r);
      n.rev = false;
    }
    if (n.lz) {
      if (n.l) aff(n.l, n.b, n.c);
      if (n.r) aff(n.r, n.b, n.c);
      n.b = Z(1), n.c = Z(0), n.lz = false;
    }
  }
  void pull(int x) {
    Node &n = t[x];
    n.sz = t[n.l].sz + 1 + t[n.r].sz, n.sum = t[n.l].sum + n.val + t[n.r].sum;
  }
  int node(int l, int k, int r) {
    t[k].l = l, t[k].r = r, pull(k);
    return k;
  }
  // l が r より重いとき、l の右の背を下りて k と r をつなぐ。
  int join_right(int l, int k, int r) {
    if (bal(w(l), w(r))) return node(l, k, r);
    push(l);
    const int a = t[l].l, x = join_right(t[l].r, k, r);
    if (bal(w(a), w(x))) return node(a, l, x);
    push(x);
    const int x1 = t[x].l, x2 = t[x].r;
    if (bal(w(a), w(x1)) && bal(w(a) + w(x1), w(x2))) return node(node(a, l, x1), x, x2);  // 左へ 1 回の回転
    push(x1);
    const int y1 = t[x1].l, y2 = t[x1].r;
    return node(node(a, l, y1), x1, node(y2, x, x2));  // 2 回の回転
  }
  int join_left(int l, int k, int r) {
    if (bal(w(l), w(r))) return node(l, k, r);
    push(r);
    const int b = t[r].r, x = join_left(l, k, t[r].l);
    if (bal(w(x), w(b))) return node(x, r, b);
    push(x);
    const int x1 = t[x].l, x2 = t[x].r;
    if (bal(w(x2), w(b)) && bal(w(x1), w(x2) + w(b))) return node(x1, x, node(x2, r, b));
    push(x2);
    const int y1 = t[x2].l, y2 = t[x2].r;
    return node(node(x1, x, y1), x2, node(y2, r, b));
  }
  int join(int l, int k, int r) {
    if (w(l) > 3 * w(r)) return join_right(l, k, r);
    if (w(r) > 3 * w(l)) return join_left(l, k, r);
    return node(l, k, r);
  }
  // 先頭の k 個を a に、残りを b に分ける。
  void split(int x, int k, int &a, int &b) {
    if (!x) return void(a = b = 0);
    push(x);
    const int l = t[x].l, r = t[x].r, ls = t[l].sz;
    if (k <= ls) {
      int p, q;
      split(l, k, p, q);
      a = p, b = join(q, x, r);
    } else {
      int p, q;
      split(r, k - ls - 1, p, q);
      a = join(l, x, p), b = q;
    }
  }
  // 最後の節を外して返し、残りを rest に入れる。
  int split_last(int x, int &rest) {
    push(x);
    if (!t[x].r) return rest = t[x].l, x;
    int r;
    const int m = split_last(t[x].r, r);
    rest = join(t[x].l, x, r);
    return m;
  }
  int merge(int a, int b) {
    if (!a || !b) return a | b;
    int rest;
    const int m = split_last(a, rest);
    return join(rest, m, b);
  }
  Z prod(int x, int l, int r) {
    if (!x || l >= r) return Z();
    if (l == 0 && r == t[x].sz) return t[x].sum;
    push(x);
    const int ls = t[t[x].l].sz;
    Z s;
    if (l < ls) s += prod(t[x].l, l, min(r, ls));
    if (l <= ls && ls < r) s += t[x].val;
    if (r > ls + 1) s += prod(t[x].r, max(l - ls - 1, 0), r - ls - 1);
    return s;
  }
  void apply(int x, int l, int r, Z b, Z c) {
    if (!x || l >= r) return;
    if (l == 0 && r == t[x].sz) return aff(x, b, c);
    push(x);
    const int ls = t[t[x].l].sz;
    if (l < ls) apply(t[x].l, l, min(r, ls), b, c);
    if (l <= ls && ls < r) t[x].val = b * t[x].val + c;
    if (r > ls + 1) apply(t[x].r, max(l - ls - 1, 0), r - ls - 1, b, c);
    pull(x);
  }
  int build(const vector<i64> &a, int lo, int hi) {
    if (lo >= hi) return 0;
    const int m = (lo + hi) / 2, x = make(Z(a[m]));
    const int l = build(a, lo, m), r = build(a, m + 1, hi);
    return node(l, x, r);
  }
  explicit Solver(const vector<i64> &a) {
    t.reserve(a.size() + 600000);
    t.push_back({0, 0, 0, Z(), Z(), Z(1), Z(), false, false});  // 0 は空の木
    root = build(a, 0, (int)a.size());
  }
  void insert(int i, i64 x) {
    int a, b;
    split(root, i, a, b);
    root = join(a, make(Z(x)), b);
  }
  void erase(int i) {
    int a, b, c;
    split(root, i, a, b);
    split(b, 1, b, c);
    root = merge(a, c);
  }
  void reverse(int l, int r) {
    if (r - l < 2) return;
    int a, b, c;
    split(root, l, a, b);
    split(b, r - l, b, c);
    tog(b);
    root = merge(merge(a, b), c);
  }
  void affine(int l, int r, i64 b, i64 c) { apply(root, l, r, Z(b), Z(c)); }
  i64 sum(int l, int r) { return prod(root, l, r).val(); }
};
