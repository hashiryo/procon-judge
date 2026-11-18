#pragma once
#include "pj.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// lib-neo の切り分け。NeoLibrary の HeavyLightDecomposition はそのままで、区間和を auto_bit と同じ BIT (_subtree.hpp の
// Fenwick と同じ足し方) で持つ。auto_bit との差が HLD を作る分、lib-neo との差が区間和の木の分になる。
struct Solver {
  HeavyLightDecomposition h;
  int n;
  vector<i64> b;
  Solver(int n_, const vector<i64> &a, const vector<int> &par) : h(par), n(n_), b(n_ + 1) {
    b[0] = 0;
    for (int v = 0; v < n; ++v) b[h.to_seq(v) + 1] = a[v];
    for (int i = 1; i <= n; ++i)
      if (const int j = i + (i & -i); j <= n) b[j] += b[i];
  }
  void add(int u, i64 x) {
    for (int i = h.to_seq(u) + 1; i <= n; i += i & -i) b[i] += x;
  }
  i64 subtree_sum(int u) {
    auto [l, r] = h.subtree(u);
    i64 s = 0;
    for (; r > l; r &= r - 1) s += b[r];
    for (; l > r; l &= l - 1) s -= b[l];
    return s;
  }
};
