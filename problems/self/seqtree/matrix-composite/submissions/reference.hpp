#pragma once
#include "common.hpp"

// 参照実装 (Library に依存しない)。節に値を 1 つ持つ treap で、区間を split で切り出す。反転は遅延で持ち、逆順の積も持つ。
struct Solver {
  struct Node {
    int l, r, sz;
    unsigned pri;
    Mat val, sum, rsum;
    bool rev;
  };
  vector<Node> t;
  int root = 0;
  unsigned rs = 88172645;
  unsigned rnd() { return rs ^= rs << 13, rs ^= rs >> 17, rs ^= rs << 5; }
  int make(const Mat &m) {
    t.push_back({0, 0, 1, rnd(), m, m, m, false});
    return (int)t.size() - 1;
  }
  void pull(int x) {
    Node &n = t[x];
    n.sz = 1 + t[n.l].sz + t[n.r].sz;
    Mat s = n.val, rv = n.val;
    if (n.l) s = mat_mul(t[n.l].sum, s), rv = mat_mul(rv, t[n.l].rsum);
    if (n.r) s = mat_mul(s, t[n.r].sum), rv = mat_mul(t[n.r].rsum, rv);
    n.sum = s, n.rsum = rv;
  }
  void tog(int x) {
    if (!x) return;
    swap(t[x].l, t[x].r), swap(t[x].sum, t[x].rsum), t[x].rev ^= 1;
  }
  void push(int x) {
    if (t[x].rev) tog(t[x].l), tog(t[x].r), t[x].rev = false;
  }
  void split(int x, int k, int &a, int &b) {
    if (!x) return void(a = b = 0);
    push(x);
    if (t[t[x].l].sz >= k) {
      int p, q;
      split(t[x].l, k, p, q);
      t[x].l = q, pull(x), a = p, b = x;
    } else {
      int p, q;
      split(t[x].r, k - t[t[x].l].sz - 1, p, q);
      t[x].r = p, pull(x), a = x, b = q;
    }
  }
  int merge(int a, int b) {
    if (!a || !b) return a | b;
    if (t[a].pri > t[b].pri) {
      push(a);
      t[a].r = merge(t[a].r, b), pull(a);
      return a;
    }
    push(b);
    t[b].l = merge(a, t[b].l), pull(b);
    return b;
  }
  explicit Solver(const vector<Mat> &a) {
    t.reserve(a.size() + 200001);
    t.push_back({0, 0, 0, 0, MatMul::ti(), MatMul::ti(), MatMul::ti(), false});  // 0 は空の木
    for (auto &m : a) root = merge(root, make(m));
  }
  void insert(int i, const Mat &m) {
    int a, b;
    split(root, i, a, b);
    root = merge(merge(a, make(m)), b);
  }
  void erase(int i) {
    int a, b, c;
    split(root, i, a, b);
    split(b, 1, b, c);
    root = merge(a, c);
  }
  void reverse(int l, int r) {
    int a, b, c;
    split(root, l, a, b);
    split(b, r - l, b, c);
    tog(b);
    root = merge(merge(a, b), c);
  }
  Mat prod(int l, int r) {
    int a, b, c;
    split(root, l, a, b);
    split(b, r - l, b, c);
    const Mat s = t[b].sum;
    root = merge(merge(a, b), c);
    return s;
  }
};
