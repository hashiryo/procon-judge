#pragma once
// 愚直解 (pj testdata crosscheck 用)。頂点が 12 個ほどまでの入力で、頂点集合を全部試して最小カットを求める。
// 木は Gusfield の方法 (reference.hpp と同じ手順) で作り、カットは「s を含み t を含まない集合」を全部試して容量の最小の
// ものを使う。作った木の道の上の最小が、全部の組で全探索の λ(s, t) に等しいことをここでも確かめ、違えば落とす (手順の
// 誤りを、参照実装と同じ誤りとして見逃さないため)。
#include <cstdlib>
#include <limits>
#include <vector>

struct Solver {
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    if (n > 16) abort();
    const int full = 1 << n;
    vector<long long> cut(full, 0);
    for (int x = 0; x < full; ++x)
      for (auto &e : edges)
        if (((x >> e[0]) & 1) != ((x >> e[1]) & 1)) cut[x] += e[2];
    auto best = [&](int s, int t) {
      int arg = -1;
      for (int x = 0; x < full; ++x)
        if ((x >> s & 1) && !(x >> t & 1) && (arg < 0 || cut[x] < cut[arg])) arg = x;
      return arg;
    };
    vector<int> p(n, 0);
    vector<long long> fl(n, 0);
    for (int s = 1; s < n; ++s) {
      const int t = p[s], X = best(s, t);
      const long long f = cut[X];
      fl[s] = f;
      for (int i = 0; i < n; ++i)
        if (i != s && (X >> i & 1) && p[i] == t) p[i] = s;
      if (t != 0 && (X >> p[t] & 1)) {
        p[s] = p[t], p[t] = s;
        fl[s] = fl[t], fl[t] = f;
      }
    }
    res.clear();
    for (int v = 1; v < n; ++v) res.push_back({v, p[v], fl[v]});
    // 木の道の上の最小と全探索の λ を突き合わせる。
    vector<vector<pair<int, long long>>> adj(n);
    for (auto &[u, v, w] : res) adj[u].push_back({(int)v, w}), adj[v].push_back({(int)u, w});
    for (int s = 0; s < n; ++s) {
      vector<long long> mn(n, -1);
      vector<int> st{s};
      mn[s] = numeric_limits<long long>::max();
      while (!st.empty()) {
        int u = st.back();
        st.pop_back();
        for (auto [v, w] : adj[u])
          if (mn[v] < 0) mn[v] = min(mn[u], w), st.push_back(v);
      }
      for (int t = 0; t < n; ++t)
        if (t != s && mn[t] != cut[best(s, t)]) abort();
    }
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
