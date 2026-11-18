#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/graph/Rerooting.hpp"

// NeoLibrary の Rerooting (csr と同じく、隣接の CSR を組んで BFS の順に並べ直してから 2 つの段を位置の順に回す形)。部分木の値は
// lib と同じ (関数を当てた和, 頂点の数) の組で、値は ZMod。
struct Solver {
  using Z = ZMod<998244353>;
  using D = pair<Z, int>;
  int n;
  vector<Z> a, b, c;
  vector<array<int, 2>> edges;
  vector<u32> ans;
  Solver(int n, const vector<u32> &a_, const vector<array<int, 2>> &e, const vector<u32> &b_, const vector<u32> &c_)
      : n(n), a(a_.begin(), a_.end()), b(b_.begin(), b_.end()), c(c_.begin(), c_.end()), edges(e), ans(n) {}
  void run() {
    auto put_edge = [&](int, int e, const D &d) { return D{b[e] * d.first + c[e] * Z(d.second), d.second}; };
    auto op = [](const D &l, const D &r) { return D{l.first + r.first, l.second + r.second}; };
    auto put_vertex = [&](int v, const D &d) { return D{d.first + a[v], d.second + 1}; };
    Rerooting<D> dp(n, edges, put_edge, op, D{Z(), 0}, put_vertex);
    for (int i = 0; i < n; ++i) ans[i] = dp[i].first.val();
  }
  const vector<u32> &answer() const { return ans; }
};
