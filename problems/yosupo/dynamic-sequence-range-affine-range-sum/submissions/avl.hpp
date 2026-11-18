#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"

// 配列の pool に節を置いた AVL 木 (どの節も値を持つ形)。左右の高さの差を 1 以下に保つ。split と併合は join (真ん中の節を 1 つ
// 挟んで 2 つの木をつなぎ、高い方の背を下りて 1 回か 2 回の回転で直す、Blelloch らの Just Join の AVL 木の形) で書く。
// 平衡のとり方のほかは wbt.hpp と同じで、区間の和と 1 次関数の作用は形を変えずに根から下りて覆う部分木に当て、反転だけ split を
// 2 回して真ん中の木に印を付け、併合で戻す。1 次関数の作用と反転は遅延で持つ。
struct Solver {
  using Z = ZMod<998244353>;
  struct Node {
    int l, r, sz, h;
    Z val, sum, b, c;
    bool rev, lz;
  };
  vector<Node> t;
  int root = 0;
  int make(Z v) {
    t.push_back({0, 0, 1, 1, v, v, Z(1), Z(0), false, false});
    return (int)t.size() - 1;
  }
  int h(int x) const { return t[x].h; }
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
    n.h = 1 + max(t[n.l].h, t[n.r].h);
  }
  int node(int l, int k, int r) {
    t[k].l = l, t[k].r = r, pull(k);
    return k;
  }
  // 子を付け替える回転。動かす子は先に遅延を配る。
  int rot_left(int x) {
    const int y = t[x].r;
    push(y);
    t[x].r = t[y].l, pull(x);
    t[y].l = x, pull(y);
    return y;
  }
  int rot_right(int x) {
    const int y = t[x].l;
    push(y);
    t[x].l = t[y].r, pull(x);
    t[y].r = x, pull(y);
    return y;
  }
  // l が r より 2 以上高いとき、l の右の背を下りて k と r をつなぐ。
  int join_right(int l, int k, int r) {
    push(l);
    const int a = t[l].l, c = t[l].r;
    if (h(c) <= h(r) + 1) {
      int x = node(c, k, r);
      if (h(x) <= h(a) + 1) return node(a, l, x);
      x = rot_right(x);
      return rot_left(node(a, l, x));
    }
    const int x = join_right(c, k, r);
    const int y = node(a, l, x);
    return h(x) <= h(a) + 1 ? y : rot_left(y);
  }
  int join_left(int l, int k, int r) {
    push(r);
    const int c = t[r].l, b = t[r].r;
    if (h(c) <= h(l) + 1) {
      int x = node(l, k, c);
      if (h(x) <= h(b) + 1) return node(x, r, b);
      x = rot_left(x);
      return rot_right(node(x, r, b));
    }
    const int x = join_left(l, k, c);
    const int y = node(x, r, b);
    return h(x) <= h(b) + 1 ? y : rot_right(y);
  }
  int join(int l, int k, int r) {
    if (h(l) > h(r) + 1) return join_right(l, k, r);
    if (h(r) > h(l) + 1) return join_left(l, k, r);
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
    t.push_back({0, 0, 0, 0, Z(), Z(), Z(1), Z(), false, false});  // 0 は空の木
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
