#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/Rerooting.hpp"

// 今の Library の Rerooting。部分木の値を (関数を当てた和, 頂点の数) の組で持つ。Graph を組むところから測る。
struct Solver {
  using Mint = ModInt<998244353>;
  using Data = pair<Mint, int>;
  int n;
  vector<Mint> a, b, c;
  vector<array<int, 2>> edges;
  vector<u32> ans;
  Solver(int n, const vector<u32> &a_, const vector<array<int, 2>> &e, const vector<u32> &b_, const vector<u32> &c_)
      : n(n), a(a_.begin(), a_.end()), b(b_.begin(), b_.end()), c(c_.begin(), c_.end()), edges(e), ans(n) {}
  void run() {
    Graph g(n, n - 1);
    for (int i = 0; i + 1 < n; ++i) g[i] = {edges[i][0], edges[i][1]};
    auto put_edge = [&](int, int e, const Data &d) { return Data{b[e] * d.first + c[e] * d.second, d.second}; };
    auto op = [&](const Data &l, const Data &r) { return Data{l.first + r.first, l.second + r.second}; };
    auto put_vertex = [&](int v, const Data &d) { return Data{d.first + a[v], d.second + 1}; };
    Rerooting<Data> dp(g, put_edge, op, Data{0, 0}, put_vertex);
    for (int i = 0; i < n; ++i) ans[i] = dp[i].first.val();
  }
  const vector<u32> &answer() const { return ans; }
};
