#pragma once
#include "_wide.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"

// lib-neo の切り分け。NeoLibrary の HeavyLightDecomposition はそのままで、区間和の木を xor_wide と同じ手書きの
// WidePrefixSum<3> (_wide.hpp) に替える。xor_wide との差が HLD の分、lib-neo-mm との差が区間和の木の分になる。
struct Solver {
  HeavyLightDecomposition h;
  WidePrefixSum<3> w;
  static vector<i64> by_pos(const HeavyLightDecomposition &h, const vector<i64> &a) {
    vector<i64> v(a.size());
    for (int u = 0; u < (int)a.size(); ++u) v[h.to_seq(u)] = a[u];
    return v;
  }
  Solver(int n, const vector<i64> &a, const vector<array<int, 2>> &edges) : h(n, edges), w(by_pos(h, a)) {}
  void add(int p, i64 x) { w.add(h.to_seq(p), x); }
  i64 path_sum(int u, int v) {
    i64 s = 0;
    h.path(u, v, [&](int l, int r) { s += w.sum(min(l, r), max(l, r) + 1); });
    return s;
  }
};
