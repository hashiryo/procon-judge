#pragma once
#include "_path.hpp"

// 根から各頂点までの和 S を、行きがけ順の位置で持つ。頂点 k に x を足すと、k の部分木の位置の区間 [pos k, pos k + 大きさ)
// の S がどれも x 増えるので、S の差分の列の BIT に 2 点で足す。u と v の道の和は S(u) + S(v) - 2 S(w) + a_w (w は lca)。
// a_w は位置ごとの値の列 val から読む。lca の求め方は Lca が、差分の列の持ち方は Sum が決める。Sum は位置 n 個の初めの値の
// 列から作れて、add(i, x) と prefix(r) ([0, r) の和) を持つ。
struct FenwickSum {
  path_hld::Fenwick f;
  FenwickSum(int n, const vector<i64> &d) : f(n, d.data()) {}
  void add(int i, i64 x) { f.add(i, x); }
  i64 prefix(int r) const { return f.prefix(r); }
};
template <class Lca, class Sum = FenwickSum> struct EulerPathSum {
  path_hld::Tree t;
  path_hld::Buf raw;
  i64 *val;
  Sum bit;
  Lca lca;
  static vector<i64> diff(const path_hld::Tree &t, i64 *val, const vector<i64> &a) {
    const int n = t.n;
    for (int u = 0; u < n; ++u) val[t.pos[u]] = a[u];
    // 位置の小さい順に、親の位置の S に足すと S が決まる。差分に直して BIT にする。
    vector<i64> s(n), d(n);
    for (int x = 0; x < n; ++x) s[x] = (x ? s[t.upp[x]] : 0) + val[x];
    for (int x = 0; x < n; ++x) d[x] = s[x] - (x ? s[x - 1] : 0);
    return d;
  }
  EulerPathSum(int n, const vector<i64> &a, const vector<array<int, 2>> &edges)
      : t(path_hld::Tree::make<path_hld::Build::Xor>(n, edges)), raw(path_hld::alloc_huge(2 * size_t(n))),
        val((i64 *)raw.get()), bit(n, diff(t, val, a)), lca(t) {}
  void add(int p, i64 x) {
    const int k = t.pos[p], e = k + t.szp[k];
    val[k] += x, bit.add(k, x);
    if (e < t.n) bit.add(e, -x);
  }
  i64 path_sum(int u, int v) {
    const int x = t.pos[u], y = t.pos[v], w = lca(x, y);
    return bit.prefix(x + 1) + bit.prefix(y + 1) - 2 * bit.prefix(w + 1) + val[w];
  }
};
