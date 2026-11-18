#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"

// 配列の pool に節を置いた treap。優先度は xorshift の乱数で、split と merge は大きさで分ける再帰。1 次関数の作用と反転を
// 遅延で持つ。区間の操作は split を 2 回して真ん中の木に当て、merge で戻す。節は構築のときに n + q 個ぶん取っておく。
struct Solver {
  using Z = ZMod<998244353>;
  struct Node {
    int l, r, sz;
    unsigned pri;
    Z val, sum, b, c;
    bool rev, lz;
  };
  vector<Node> t;
  int root = 0;
  unsigned rs = 2463534242u;
  unsigned rnd() { return rs ^= rs << 13, rs ^= rs >> 17, rs ^= rs << 5; }
  int make(Z v) {
    t.push_back({0, 0, 1, rnd(), v, v, Z(1), Z(0), false, false});
    return (int)t.size() - 1;
  }
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
  // 先頭の k 個を a に、残りを b に分ける。
  void split(int x, int k, int &a, int &b) {
    if (!x) return void(a = b = 0);
    push(x);
    if (t[t[x].l].sz >= k) {
      int l;
      split(t[x].l, k, a, l);
      t[x].l = l, b = x;
    } else {
      int r;
      split(t[x].r, k - t[t[x].l].sz - 1, r, b);
      t[x].r = r, a = x;
    }
    pull(x);
  }
  int merge(int a, int b) {
    if (!a || !b) return a | b;
    if (t[a].pri > t[b].pri) {
      push(a);
      const int r = merge(t[a].r, b);
      t[a].r = r, pull(a);
      return a;
    }
    push(b);
    const int l = merge(a, t[b].l);
    t[b].l = l, pull(b);
    return b;
  }
  explicit Solver(const vector<i64> &a) {
    t.reserve(a.size() + 600000);
    t.push_back({0, 0, 0, 0, Z(), Z(), Z(1), Z(), false, false});  // 0 は空の木
    for (i64 v : a) root = merge(root, make(Z(v)));
  }
  void insert(int i, i64 x) {
    int a, b;
    split(root, i, a, b);
    root = merge(merge(a, make(Z(x))), b);
  }
  void erase(int i) {
    int a, b, c;
    split(root, i, a, b);
    split(b, 1, b, c);
    root = merge(a, c);
  }
  template <class F> void range(int l, int r, const F &f) {
    int a, b, c;
    split(root, l, a, b);
    split(b, r - l, b, c);
    f(b);
    root = merge(merge(a, b), c);
  }
  void reverse(int l, int r) {
    range(l, r, [&](int x) {
      if (x) tog(x);
    });
  }
  void affine(int l, int r, i64 b, i64 c) {
    range(l, r, [&](int x) {
      if (x) aff(x, Z(b), Z(c));
    });
  }
  i64 sum(int l, int r) {
    Z s;
    range(l, r, [&](int x) { s = t[x].sum; });
    return s.val();
  }
};
