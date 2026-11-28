#pragma once
// 期待出力を作る参照実装。永続セグメント木を NeoLibrary を使わずに素直に書いたもの。
// 愚直解 (brute.hpp) と小さい入力で突き合わせる (pj testdata crosscheck)。
#include <vector>

struct Solver {
  struct Node {
    int l = 0, r = 0;
    long long v = 0;
  };
  std::vector<Node> t;
  std::vector<int> root;
  int n = 0, sz = 1;

  explicit Solver(const std::vector<long long> &a, int) {
    n = a.size(), sz = 1;
    while (sz < (n > 0 ? n : 1)) sz <<= 1;
    t.push_back(Node{});
    root.push_back(build(0, sz, a));
  }
  int build(int l, int r, const std::vector<long long> &a) {
    int p = t.size();
    t.push_back(Node{});
    if (r - l == 1) {
      t[p].v = l < n ? a[l] : 0;
      return p;
    }
    int m = (l + r) >> 1;
    int lc = build(l, m, a), rc = build(m, r, a);
    t[p].l = lc, t[p].r = rc, t[p].v = t[lc].v + t[rc].v;
    return p;
  }
  int set_rec(int p, int l, int r, int i, long long x) {
    int q = t.size();
    t.push_back(t[p]);
    if (r - l == 1) {
      t[q].v = x;
      return q;
    }
    int m = (l + r) >> 1;
    if (i < m) t[q].l = set_rec(t[p].l, l, m, i, x);
    else t[q].r = set_rec(t[p].r, m, r, i, x);
    t[q].v = t[t[q].l].v + t[t[q].r].v;
    return q;
  }
  long long sum_rec(int p, int l, int r, int ql, int qr) const {
    if (qr <= l || r <= ql) return 0;
    if (ql <= l && r <= qr) return t[p].v;
    int m = (l + r) >> 1;
    return sum_rec(t[p].l, l, m, ql, qr) + sum_rec(t[p].r, m, r, ql, qr);
  }
  void set(int v, int i, long long x) { root.push_back(set_rec(root[v], 0, sz, i, x)); }
  long long sum(int v, int l, int r) const { return sum_rec(root[v], 0, sz, l, r); }
};
