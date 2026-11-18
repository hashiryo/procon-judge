#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"

// 配列の pool に節を置いた splay 木 (Library Checker の chaihf の 278236 と同じ考え方)。親を持ち、根から k 番目へ下りながら遅延を
// 配ってから、下から回転で根へ上げる (再帰しないので、木が深くても積み上がらない)。両端に番兵を置き、区間 [l, r) は、l 番目
// (番兵込み) を根に、r + 1 番目を根の右の子に上げたときの、その左の部分木になる。1 次関数の作用と反転を遅延で持つ。
struct Solver {
  using Z = ZMod<998244353>;
  struct Node {
    int ch[2], p, sz;
    Z val, sum, b, c;
    bool rev, lz;
  };
  vector<Node> t;
  int root = 0;
  int make(Z v) {
    t.push_back({{0, 0}, 0, 1, v, v, Z(1), Z(0), false, false});
    return (int)t.size() - 1;
  }
  void aff(int x, Z b, Z c) {
    Node &n = t[x];
    n.val = b * n.val + c, n.sum = b * n.sum + c * Z(n.sz);
    n.b = b * n.b, n.c = b * n.c + c, n.lz = true;
  }
  void tog(int x) { swap(t[x].ch[0], t[x].ch[1]), t[x].rev ^= 1; }
  void push(int x) {
    Node &n = t[x];
    if (n.rev) {
      if (n.ch[0]) tog(n.ch[0]);
      if (n.ch[1]) tog(n.ch[1]);
      n.rev = false;
    }
    if (n.lz) {
      if (n.ch[0]) aff(n.ch[0], n.b, n.c);
      if (n.ch[1]) aff(n.ch[1], n.b, n.c);
      n.b = Z(1), n.c = Z(0), n.lz = false;
    }
  }
  void pull(int x) {
    Node &n = t[x];
    n.sz = t[n.ch[0]].sz + 1 + t[n.ch[1]].sz, n.sum = t[n.ch[0]].sum + n.val + t[n.ch[1]].sum;
  }
  void rotate(int x) {
    const int y = t[x].p, z = t[y].p, d = t[y].ch[1] == x, w = t[x].ch[d ^ 1];
    t[y].ch[d] = w;
    if (w) t[w].p = y;
    t[x].ch[d ^ 1] = y, t[y].p = x, t[x].p = z;
    if (z) t[z].ch[t[z].ch[1] == y] = x;
    pull(y);
  }
  // x を goal の子 (goal が 0 なら根) まで上げる。x から根までの遅延は配ってあるとする。
  void splay(int x, int goal) {
    while (t[x].p != goal) {
      const int y = t[x].p, z = t[y].p;
      if (z != goal) rotate((t[z].ch[1] == y) == (t[y].ch[1] == x) ? y : x);
      rotate(x);
    }
    pull(x);
    if (!goal) root = x;
  }
  // 番兵込みで k 番目の節。下りながら遅延を配る。
  int kth(int k) {
    int x = root;
    for (;;) {
      push(x);
      const int ls = t[t[x].ch[0]].sz;
      if (k < ls) x = t[x].ch[0];
      else if (k == ls) return x;
      else k -= ls + 1, x = t[x].ch[1];
    }
  }
  // 区間 [l, r) (番兵を除いた位置) を、根の右の子の左の部分木に集めて、その部分木の根を返す。
  int gather(int l, int r) {
    splay(kth(l), 0);
    splay(kth(r + 1), root);
    return t[t[root].ch[1]].ch[0];
  }
  int build(const vector<Z> &v, int lo, int hi, int p) {
    if (lo >= hi) return 0;
    const int m = (lo + hi) / 2, x = make(v[m]);
    t[x].p = p;
    const int l = build(v, lo, m, x), r = build(v, m + 1, hi, x);
    t[x].ch[0] = l, t[x].ch[1] = r;
    pull(x);
    return x;
  }
  explicit Solver(const vector<i64> &a) {
    t.reserve(a.size() + 600000);
    t.push_back({{0, 0}, 0, 0, Z(), Z(), Z(1), Z(), false, false});  // 0 は空
    vector<Z> v(a.size() + 2);
    for (size_t i = 0; i < a.size(); ++i) v[i + 1] = Z(a[i]);
    root = build(v, 0, (int)v.size(), 0);  // 両端は番兵 (値 0)
  }
  void insert(int i, i64 x) {
    splay(kth(i), 0);
    splay(kth(i + 1), root);
    const int y = t[root].ch[1], z = make(Z(x));
    t[y].ch[0] = z, t[z].p = y;
    pull(y), pull(root);
  }
  void erase(int i) {
    const int x = gather(i, i + 1), y = t[root].ch[1];
    t[y].ch[0] = 0, t[x].p = 0;
    pull(y), pull(root);
  }
  void reverse(int l, int r) {
    if (r - l < 2) return;
    const int x = gather(l, r);
    tog(x);
  }
  void affine(int l, int r, i64 b, i64 c) {
    if (l == r) return;
    const int x = gather(l, r), y = t[root].ch[1];
    aff(x, Z(b), Z(c));
    pull(y), pull(root);
  }
  i64 sum(int l, int r) {
    if (l == r) return 0;
    return t[gather(l, r)].sum.val();
  }
};
